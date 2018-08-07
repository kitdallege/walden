#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <fcntl.h>                                                                                                                                                                           
#include <sys/types.h>
#include <sys/stat.h>                                                                                                                                                                        
#include <unistd.h>                                                                                                                                                                          
#include <linux/limits.h> 
#include <sys/inotify.h>
#include <time.h>

#include <libpq-fe.h>

#include "sha1/sha1.h"
#include "handlers.h"
#include "config.h"
#include "walker.h"

//-------------------------------------------------------------------------
// Types & Prototypes
//-------------------------------------------------------------------------
typedef struct inotify_event InotifyEvent;

// FileEventQueue is a FileEvent w/ a *next pointer (to act as a linked-list)
typedef struct FileEventQueue {
	union {
		struct FileEvent;
		FileEvent event;
	};
	struct FileEventQueue *next;
} FileEventQueue;

struct Handler
{
	long update_interval;
	long last_update;
	Config *conf;
	PGconn *conn;
	FileEventQueue *queue;
};

static void handler_drain_queue(Handler *self, long current_nanos);
static void handler_add_or_mod_file(Handler *self, const char *filepath);
static void handler_del_file(Handler *self, const char *filepath);
static char *compute_sha1(const char *filepath);
static char *string_from_file(const char *filepath);
static long get_nanos(void);

//-------------------------------------------------------------------------
// Public API
//-------------------------------------------------------------------------
Handler *handler_aloc()
{
	Handler *self = calloc(1, sizeof *self);
	self->conn  = NULL;
	return self;
}

void handler_conf(Handler *self, void *user)
{
	self->update_interval = 5000 * 1000000L; // in msec 
	self->last_update = get_nanos();

	self->conf = (Config *)user;
	fprintf(stderr, "Connecting to: %s\n", self->conf->db_conn_info);
	self->conn = PQconnectdb(self->conf->db_conn_info);
	if (PQstatus(self->conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s",
				PQerrorMessage(self->conn));
		return ;
	}
	fprintf(stderr, "db conn successful. \n");
	// TODO: setup prepared statements.
	PGresult *res = PQexec(self->conn, "set search_path = walden"); 
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "SET failed: %s\n", PQerrorMessage(self->conn));
		PQclear(res);
	}
	PQclear(res);
	fprintf(stderr, "Set search_path = walden successful.\n");

}

void handler_step(Handler *self, void *user)
{
	// check delta to clamp update hz 
	long current = get_nanos();
	if ((current - self->last_update) > self->update_interval) {
		handler_drain_queue(self, current);
		self->last_update = current;
	}
}

void handler_zero(Handler *self)
{
	//self->conf->db_conn_info = NULL;
	// while evt in queue; free(evt)
	// PQexec(conn, "DEALLOCATE %s" % stmtName)
	PQfinish(self->conn);
}

void handler_free(Handler *self)
{
	//free(self->event_queue);
	free(self);
}

// TODO: check queue for item before enqueing / eg: dedup
void handler_enqueue_event(Handler *self, FileEvent *event)
{
	fprintf(stderr, "handler_enqueue_event: %s \n", event->filename);
	FileEventQueue *feq = calloc(1, sizeof *feq);
	feq->event = *event;
	if (self->queue) {
		feq->next = self->queue;
	}
	self->queue = feq;
}

// TODO: atm this is O(N) just run a query for every found file. 
// TODO: api changed... get_or_create(site_id integer, fullpath text, checksum text) 
// select get_or_create($1::integer, $2::text, $3::text);
#define TEMPLATE_CREATE_OR_UPDATE_SQL "select walden_template_get_or_create($1::integer, $2::text, $3::text);"
#define QUERY_CREATE_OR_UPDATE_SQL "select walden_query_get_or_create($1::integer, $2::text, $3::text);"
void handler_sync_all(Handler *self)
{
	PGresult *res;
	Files *templates = find_files(self->conf->template_root);
	for (int i = 0, len = templates->count; i < len; i++) {
		// cut template_root off the path
		char *path = templates->paths[i];
		char *checksum = compute_sha1(path);
		char *params[3];
		// adding 1 to template_root to take the starting '/' as well
		params[0] = self->conf->site_id;
		params[1] = path + strlen(self->conf->template_root) + 1;
		params[2] = checksum; 
		fprintf(stderr, "path: %s site_id:%s rel: %s checksum: %s\n", path, params[0], params[1], params[2]);
		res = PQexecParams(
			self->conn,
			TEMPLATE_CREATE_OR_UPDATE_SQL,
			3, NULL, (const char* const*)params, NULL, NULL, 0
		);
		if (PQresultStatus(res) != PGRES_TUPLES_OK) {
			fprintf(
				stderr, "template_create_or_update failed! error: %s\n",
				PQerrorMessage(self->conn)
			);
		}
		PQclear(res);
		free(checksum);
	}
	free_files(templates);

	Files *queries = find_files(self->conf->query_root);
	for (int i = 0, len = queries->count; i < len; i++) {
		// cut template_root off the path
		char *path = queries->paths[i];
		char *checksum = compute_sha1(path);	
		char *params[2];
		params[0] = self->conf->site_id;
		params[1] = path + strlen(self->conf->query_root) + 1;
		params[2] = checksum; 
		fprintf(stderr, "path: %s site_id:%s rel: %s checksum: %s\n", path, params[0], params[1], params[2]);
		res = PQexecParams(
			self->conn,
			QUERY_CREATE_OR_UPDATE_SQL,
			3, NULL, (const char * const*)params, NULL, NULL, 0
		);
		if (PQresultStatus(res) != PGRES_TUPLES_OK) {
			fprintf(
				stderr, "query_create_or_update failed! error: %s\n",
				PQerrorMessage(self->conn)
			);
		}
		PQclear(res);
		free(checksum);
	}
	free_files(queries);
}

//-------------------------------------------------------------------------
// Private API
//-------------------------------------------------------------------------
static void handler_drain_queue(Handler *self, long current_nanos)
{
	if (self->queue) {
		fprintf(stderr, "handler_drain_queue drainring\n");
		FileEventQueue *temp, *feq = self->queue;
	    do {
			// process queued event
			switch (feq->event.type) {
				case FET_MOD:
				case FET_ADD:
					fprintf(stderr, "add event for file: %s\n", feq->filename);
					handler_add_or_mod_file(self, feq->filename);
					break;
				case FET_DEL:
					fprintf(stderr, "del event for file: %s\n", feq->filename);
					handler_del_file(self, feq->filename);
					break;
				default:
					fprintf(stderr, "unknown event for file: %s\n", feq->filename);
					break;
			}
			// get next item in queue and free this one.
			temp = feq;
			feq = feq->next;
			fprintf(stderr, "freeing: %s \n", temp->filename);
			free(temp->filename);
			free(temp);
		} while (feq);
		self->last_update = current_nanos;
		// TODO: got the feeling were leaving a man behind. thus the NULL hack
		self->queue = NULL;
	}
}

//TODO: create add_or_update functions for both query & template tables
// then combine the two handlers below into one.
static void handler_add_or_mod_file(Handler *self, const char *path)
{
	const char *stmt;
	if (strstr(path, self->conf->template_root)) {
		stmt = TEMPLATE_CREATE_OR_UPDATE_SQL;
	} else {
		stmt = QUERY_CREATE_OR_UPDATE_SQL;
	}
	char *checksum = compute_sha1(path);
	const char *params[2];
	params[0] = path + strlen(self->conf->template_root) + 1;
	params[1] = checksum;

	PGresult *result;
	result = PQexecParams(
		self->conn,
		TEMPLATE_CREATE_OR_UPDATE_SQL,
		2, NULL, params, NULL, NULL, 0
	);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		fprintf(
			stderr, "query failed: error:%s stmt:%s\n",
			PQerrorMessage(self->conn), stmt
		);
	}
	PQclear(result);
	free(checksum);
}
//---------------------------------------------------------------------------
// TODO: both template & query should have a deleted flag(s) so that we can
// leave the record intact and use it in quries to determine what relations
// have been (are currentl) broken by change.	
//---------------------------------------------------------------------------
#define TEMPLATE_DELETE_SQL "select template_delete_by_fullpath($1);"
#define QUERY_DELETE_SQL "select query_delete_by_fullpath($1);"
static void handler_del_file(Handler *self, const char *path)
{
	const char *stmt;
	if (strstr(path, self->conf->template_root)) {
		stmt = TEMPLATE_DELETE_SQL;
	} else {
		stmt = QUERY_DELETE_SQL;
	}
	const char *params[1];
	params[0] = path + strlen(self->conf->template_root) + 1;
	PGresult *result;
	result = PQexecParams(
		self->conn,
		TEMPLATE_CREATE_OR_UPDATE_SQL,
		1, NULL, params, NULL, NULL, 0
	);
	if (PQresultStatus(result) != PGRES_TUPLES_OK) {
		fprintf(
			stderr, "query failed: error:%s stmt:%s\n",
			PQerrorMessage(self->conn), stmt
		);
	}
	PQclear(result);
}

//-------------------------------------------------------------------------
// Utils 
//-------------------------------------------------------------------------
static char *compute_sha1(const char *filepath)
{
	size_t offset;
	char result[21];
	char hexresult[41];
 	char *data = string_from_file(filepath);
	if (!data) {
		fprintf(stderr, "unable to load string from file: %s\n", filepath);
		return NULL;
	}
	// TODO: Why in the FUCK does SHA1 collide (it has to be the dynamic loader
	// shit) cuz no where else are we linking in libcrypto.
	mk_checksum(result, data, strlen(data));
	for(offset = 0; offset < 20; offset++) {
		sprintf( (hexresult + (2 * offset)), "%02x", result[offset] & 0xff);
	}
	free(data);
	// caller is responsible for free'ing hexresult
	return strdup(hexresult);
}

static char *string_from_file(const char *filepath)
{
	char *data;
	struct stat st;
	int fd;
	fprintf(stderr, "open: %s\n", filepath);
	fd = open(filepath, O_RDONLY);
	fstat(fd, &st);
	data = calloc(1, st.st_size + 1);
	memset(data, 0, st.st_size + 1);
	read(fd, data, st.st_size);
	close(fd);
	return data;
} 	

static long get_nanos(void)
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ts.tv_sec * 1000000000L + ts.tv_nsec;
}


#include <stdlib.h>
#include <stdio.h>

#include <sys/inotify.h>
#include <libpq-fe.h>

#include "handlers.h"
//#include "core.h"
//#include "watcher.h"

//static void add_watch(AppState *state, char *dirname);
typedef struct inotify_event InotifyEvent;

struct Handler
{
	const char *db_conn_info;
	PGconn *conn;
};

Handler *handler_aloc()
{
	Handler *self = calloc(1, sizeof *self);
	self->conn  = NULL;
	return self;
}

void handler_conf(Handler *self, void *user)
{
	self->db_conn_info = (const char *)user;
	PGresult *res;
	fprintf(stderr, "Connecting to: %s\n", self->db_conn_info);
	self->conn = PQconnectdb(self->db_conn_info);
	if (PQstatus(self->conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s",
				PQerrorMessage(self->conn));
		return ;
	}
	// set search_path
	res = PQexec(self->conn, "set search_path = c2v");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "set search_path failed: %s\n",
				PQerrorMessage(self->conn));
		PQclear(res);
		return ;
	}
	PQclear(res);
	fprintf(stderr, "db conn successful. \n");
}

void handler_step(Handler *self, void *user)
{
	InotifyEvent *evt = (InotifyEvent *)user;
	int mask = evt->mask & (IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED);
	fprintf(
		stderr,
		"inotify_event wd:%d mask:%d cookie:%d len:%d name:%s \n",
		evt->wd, evt->mask, evt->cookie, evt->len, evt->name
	);
	fprintf(stderr, "switch value: %#010x \n", mask);
	switch (mask) {
		case IN_ISDIR: {
			// add watch
			fprintf(stderr, "IN_ISDIR\n");
			//add_watch(state, evt->name);
			break;
		}
		case IN_CLOSE_WRITE:
			fprintf(stderr, "IN_CLOSE_WRITE\n");
			break;
		case IN_CREATE:
			fprintf(stderr, "IN_CREATE \n");
			break;
		case IN_DELETE:
			fprintf(stderr, "IN_DELETE \n");
		default:
			fprintf(stderr, "unhandled event type\n");
			break;
	}
}

void handler_zero(Handler *self)
{

}

void handler_free(Handler *self)
{

}

/*
static void add_watch(AppState *state, char *dirname)
{
	// TODO: check for existing watch for dir ?
	int wd = inotify_add_watch(
		state->fd, dirname,
		IN_CLOSE_WRITE | IN_MOVE | IN_DELETE | IN_ATTRIB
	);
	Watch *watch = malloc(sizeof *watch);
	*watch = (Watch){.wd=wd, .path=dirname};
	watch->next = state->watches;
	state->watches = watch;
}
*/



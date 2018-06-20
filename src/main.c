/*
 * Program which renders a webpage when told via postgres (listen/notify).
 *
 * Listens for 'dirty_webpage' and parses the json payload it contains
 * into a struct.
 *
 * Combines template & query to produce a page, also writes a pjax file
 * "which is the contents of <main></main> served under /_/path/to/page.html".
 *
 * Configs:
 *   - root_dir:      Root path which the others reside within.
 *   - template_dir
 *   - web_dir
 *   - query_dir
 * 
 * TODO: 
 *  - config file for settings rather than being hard coded!
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/select.h>

#include <libpq-fe.h>

#include "renderer.h"
#include "flag_flipper.h"
#include "controller.h"
#include "log.h"

//#define CONN_INFO "port=5555 dbname=test-db user=kit"
#define CONN_INFO "port=5432 dbname=c2v user=c2v_admin"
#define LISTEN_CMD_1 "listen webpage_dirty"
#define LISTEN_CMD_2 "listen webpages_dirty"

#define GET_SPEC_IDS_SQL "select distinct page_spec_id "\
		"from webpage where dirty = true;"

#define CHUNK_SIZE 2000
#define GET_DIRTY_SQL  "select p.id, p.name || '.html' as filename, "\
		"replace(p.parent_path::text, '.', '/') as path, "\
		"spec.template, spec.query, p.query_params "\
	"from webpage as p join page_spec as spec on spec.id = p.page_spec_id "\
	"where p.dirty = true and p.page_spec_id = $1 and p.id > $2 "\
	"order by p.page_spec_id, p.taxon_id, p.id "\
	"limit 2000; "\

// Global State 
// TODO: Should live in a struct instance probably.
static int quit = 0;
static PGconn *conn;
static FlagFlipperState *state;
pthread_t tid;

static void handle_signal(int signal)
{
	fprintf(stderr, "\n Caught Signal: %d \n", signal);
	PQfinish(conn);
	fprintf(stderr, "size: %lu, active: %d : ",
			bqueue_size(state->wq), state->ctl->active);
	pthread_cond_signal(&(state->ctl->cond));
	fprintf(stderr, "calling controller_deactivate: \n");
	controller_deactivate(state->ctl);
	fprintf(stderr, "called controller_deactivate: \n");
	fprintf(stderr, "calling pthread_join: \n");
	pthread_join(tid, NULL);
	fprintf(stderr, "called pthread_join: \n");
	exit(1);
}

static void exit_nicely(void)
{
	PQfinish(conn);
	exit(1);
}

static int init_postgres(void)
{
	PGresult *res;
	const char *conninfo = CONN_INFO; 
	// connect to db	
	conn = PQconnectdb(conninfo);
	if (PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s",
				PQerrorMessage(conn));
		return -1;
	}
	// set search_path	
	res = PQexec(conn, "set search_path = c2v"); 
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "set search_path failed: %s\n", PQerrorMessage(conn));
		PQclear(res);
		return -1;
	}
	PQclear(res);
	// setup listen command's
	res = PQexec(conn, LISTEN_CMD_1);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "listen command failed: %s\n", PQerrorMessage(conn));
		PQclear(res);
		return -1;
	}
	PQclear(res);
	res = PQexec(conn, LISTEN_CMD_2);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "listen command failed: %s\n", PQerrorMessage(conn));
		PQclear(res);
		return -1;
	}
	PQclear(res);
	// setup prepared statements
	res = PQprepare(conn, "get-dirty-spec-ids", GET_SPEC_IDS_SQL, 1, NULL);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "failed to prepare statement: %s\n",
				PQerrorMessage(conn));
		PQclear(res);
		return -1;
	}
	PQclear(res);
	res = PQprepare(conn, "get-dirty-pages", GET_DIRTY_SQL, 1, NULL);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "failed to prepare statement: %s\n",
				PQerrorMessage(conn));
		PQclear(res);
		return -1;
	}
	PQclear(res);
	return 0;
}

static int init(void)
{
	signal(SIGINT, handle_signal); 
	if (init_postgres()) {
		fprintf(stderr, "init_postgres failed.\n");
		return -1;
	}

	// make background worker for updating diry flags.
	state = flag_flipper_new();
	controller_activate(state->ctl);	
	if (pthread_create(&tid, NULL, webpage_clear_dirty_thread, (void *)state)) {
		fprintf(stderr, "unable to start worker thread. \n");
		return -1;
	}
	quit = 0;
	return 0;
}

static void single_page(PGnotify *notify) 
{
	timespec ct1, ct2, pt1, pt2, td1, td2;
	clock_gettime(CLOCK_MONOTONIC, &ct1);
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt1);
	if (handle_page(conn, state, notify->extra)) {
		fprintf(stderr, "handle_page error on: %s \n",
				notify->extra);
	} else {
		clock_gettime(CLOCK_MONOTONIC, &ct2);
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt2);
		//fprintf(stderr, "updated page: %s \n", notify->extra);
		td1 = diff(ct1, ct2);
		td2 = diff(pt1, pt2);
		fprintf(stderr, "%s system time elapsed: %.3f msec / %ld ns cpu time elapsed: %.3f msec / %ld ns\n ",
				get_formatted_time(),
				td1.tv_nsec / 1000000.0, td1.tv_nsec,
				td2.tv_nsec / 1000000.0, td2.tv_nsec); 

	}
	PQfreemem(notify);
	pthread_cond_broadcast(&state->ctl->cond);
}

static void multi_page(PGnotify *notify)
{
	fprintf(stderr, "multi_page START: %s\n", get_formatted_time()); 
	const char *params[2];
	PGresult *res;
	res = PQexecPrepared(conn, "get-dirty-spec-ids", 0, NULL, NULL, NULL, 0);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "get-dirty-spec-ids error: %s \n", PQerrorMessage(conn));
	}
	int spec_ids_len = PQntuples(res);
	char *spec_ids[spec_ids_len];
	for (int i=0; i < spec_ids_len; i++) {
		spec_ids[i] = PQgetvalue(res, i, 0);
	}

	for (int i = 0; i < spec_ids_len; i++) {
		int res_cnt = 0;
		params[0] = spec_ids[i];
		params[1] = "0";
		bool has_more = true;
		while (has_more) {
			PGresult *res2 = PQexecPrepared(conn, "get-dirty-pages", 2, params, NULL, NULL, 0);
			if (PQresultStatus(res2) != PGRES_TUPLES_OK) {
				fprintf(stderr, "get-dirty-spec-ids params: %s, error: %s \n", *params, PQerrorMessage(conn));
				break;
			}
			//fprintf(stdout, "res_cnt: %d\n", res_cnt);
			int res_len = PQntuples(res2);
			res_cnt += res_len;
			has_more = res_len == CHUNK_SIZE;
			if (has_more) {
				// TODO: free params[1]
				params[1] = strdup(PQgetvalue(res2, res_len-1, 0));
			}
			//fprintf(stderr, "has_more: %d, params[1]: %s\n", has_more, params[1]);
			// TODO: this becomes a queue write when we go multi-threaded.
			handle_pages(conn, state, res2, atoi(spec_ids[i]));
			// TODO: figure out column/row
			//fprintf(stderr, "step of %d items %s\n", CHUNK_SIZE, get_formatted_time()); 
		}
	}
	PQclear(res);
	//fprintf(stderr, "updated page: %s \n", notify->extra);
	fprintf(stderr, "multi_page END: %s\n", get_formatted_time()); 
}

static void run_loop(void)
{
	PGnotify *notify;
	while (!quit) {
		int sock = PQsocket(conn);
		fd_set input_mask;

		if (sock < 0) { break; }
		FD_ZERO(&input_mask);
		FD_SET(sock, &input_mask);

		if (select(sock + 1, &input_mask, NULL, NULL, NULL) < 0) {
			fprintf(stderr, "select() failed: %s\n", strerror(errno));
			exit_nicely();
		}
		PQconsumeInput(conn);
		while ((notify = PQnotifies(conn))) {
			fprintf(stderr, "notification: relname: %s \n", notify->relname);
			fprintf(stderr, "notify START: %s\n", get_formatted_time()); 
			if (!strcmp(notify->relname, "webpage_dirty")) {
				single_page(notify);
			} else if(!strcmp(notify->relname, "webpages_dirty")) {
				multi_page(notify);
			} else {
				fprintf(stderr, "notify unknown relname: %s\n",
						notify->relname);	
			}
			PQfreemem(notify);
			fprintf(stderr, "notify END: %s\n", get_formatted_time()); 
		}
		// purge the leftovers out of the queue.
		pthread_cond_signal(&state->ctl->cond);
	}
}

static void clean_up(void)
{
	fprintf(stderr, "clean_up start.\n");
	controller_deactivate(state->ctl);
	pthread_join(tid, NULL);
	fprintf(stderr, "pthread_join finished.\n");
	PQfinish(conn);
	fprintf(stderr, "clean_up finish.\n");
}

int main(int argc, char **argv)
{
	if (init()) {
		fprintf(stderr, "init failed!\n");
		exit_nicely();
		return 1;
	}
	run_loop();
	clean_up();
	return 0;
}


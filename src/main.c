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
#include <time.h>

#include <libpq-fe.h>

#include "renderer.h"
#include "flag_flipper.h"
#include "controller.h"

//#define CONN_INFO "port=5555 dbname=test-db user=kit"
#define CONN_INFO "port=5432 dbname=c2v user=c2v_admin"
#define LISTEN_CMD "listen dirty_webpage"
static int quit = 0;
static PGconn *conn;
static FlagFlipperState *state;
pthread_t tid;
typedef struct timespec timespec;

static timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

static char* get_formatted_time(void)
{

    time_t rawtime;
    struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Must be static, otherwise won't work
    static char _retval[20];
    strftime(_retval, sizeof(_retval), "%Y-%m-%d %H:%M:%S", timeinfo);

    return _retval;
}

static void handle_signal(int signal)
{
	fprintf(stderr, "\n Caught Signal: %d \n", signal);
	PQfinish(conn);
	fprintf(stderr, "size: %lu, force: %d, active: %d : ",
			bqueue_size(state->wq), state->ctl->force, state->ctl->active);
	// f'trying to determine its state. just cancel the damn thing and bail.
	//pthread_cancel(tid);
	if (state->ctl->force) {
		fprintf(stderr, "unforcing\n");
		controller_unforce(state->ctl);
		fprintf(stderr, "unforced");
	} else {
		fprintf(stderr, "controller_force \n");
		controller_force(state->ctl);
		fprintf(stderr, "controller_forced \n");
		pthread_cond_signal(&(state->ctl->cond));
	}
	fprintf(stderr, "calling controller_deactivate: \n");
	controller_deactivate(state->ctl);
	fprintf(stderr, "called controller_deactivate: \n");
	fprintf(stderr, "calling pthread_join: \n");
	pthread_join(tid, NULL);
	fprintf(stderr, "called pthread_join: \n");
	//free(state);
	exit(1);
}

static void exit_nicely(void)
{
	PQfinish(conn);
	exit(1);
}

int main(int argc, char **argv)
{
	const char *conninfo = CONN_INFO; 
	PGresult *res;
	PGnotify *notify;

	signal(SIGINT, handle_signal);
	conn = PQconnectdb(conninfo);

	if (PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s",
				PQerrorMessage(conn));
		exit_nicely();
	}

	//res = PQexec(conn, "select pg_catalog.set_config('search_path', 'public', false)");
	res = PQexec(conn, "set search_path = public"); 
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "SET failed: %s\n", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely();
	}
	PQclear(res);

	res = PQexec(conn, LISTEN_CMD);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "Listen command failed: %s\n",
				PQerrorMessage(conn));
		PQclear(res);
		exit_nicely();
	}
	PQclear(res);
	
	/* make our background worker 
	 * pass in a controller so the thread can be shutdown externally.
	 */
	state = flag_flipper_new();
	controller_activate(state->ctl);	
	if (pthread_create(&tid, NULL, webpage_clear_dirty_thread, (void *)state)) {
		fprintf(stderr, "unable to start worker thread. \n");
		quit = 1;
	}

	while (!quit) {
		int sock = PQsocket(conn);
		fd_set input_mask;

		if (sock < 0) { break; }
		FD_ZERO(&input_mask);
		FD_SET(sock, &input_mask);

		//controller_unforce(&state->ctl);

		if (select(sock + 1, &input_mask, NULL, NULL, NULL) < 0) {
			fprintf(stderr, "select() failed: %s\n", strerror(errno));
			exit_nicely();
		}
		PQconsumeInput(conn);
		int count = 0;
		while ((notify = PQnotifies(conn))) {
			//fprintf(stderr, "ASYNC NOTIFY of '%s' received from backend PID %d with a payload of: %s\n", notify->relname, notify->be_pid, notify->extra);
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
			if (count++ % 1000 == 0) {
				//fprintf(stderr, "broadcast: module 1k\n");
				pthread_cond_broadcast(&state->ctl->cond);
			}
		}
		fprintf(stderr, "broadcast: outside of notify loop\n");
		controller_force(state->ctl);
		pthread_cond_signal(&state->ctl->cond);
		//fprintf(stderr, "broadcast: sent signal \n");
	}
	controller_deactivate(state->ctl);
	pthread_join(tid, NULL);
	fprintf(stderr, "Done.\n");
	PQfinish(conn);
	return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/epoll.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "app.h"
#include "ini.h"

#define EPOLL_WAIT_MS 1000 
#define CONF_FILE "./resource-mgr.conf"
#define CONN_INFO "port=5432 dbname=c2v user=c2v_admin"
#define MAX_EVENTS 64
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN    (1024 * (EVENT_SIZE + 16))

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

static AppState *app_create(void)
{
	AppState *state = malloc(sizeof(*state));
	state->config = malloc(sizeof(state->config));
	state->buffer = calloc(1, BUF_LEN);
	state->ev = calloc(MAX_EVENTS, sizeof(*state->ev));
	fprintf(stderr, "app_create(state: %p)\n", (void *)state);
	return state;
}

static void app_delete(AppState *state)
{
	fprintf(stderr, "app_delete(state: %p)\n", (void *)state);
	PQfinish(state->conn);
	free(state->config->db_conn_info);
	free(state->config);
	free(state->buffer);
	free(state->ev);
	free(state);
	state = NULL;
}

static void app_unload(AppState *state)
{
	fprintf(stderr, "app_unload(state: %p)\n", (void *)state);
	// stop any notifications
	// release state->config // set conf = NULL;
	// close postgres connection
	// close(state->efd); // epoll
	// close(state->fd);  // inotify
	free(state->config->db_conn_info);
	PQfinish(state->conn);
}

static void app_reload(AppState *state)
{
	fprintf(stderr, "app_reload(state: %p)\n", (void *)state);
	// load conf and set state->config
	state->config->db_conn_info = ini_get_db_conf_from_file(CONF_FILE);
	if (!state->config->db_conn_info) {
		fprintf(stderr, "error loading config file.\n");
	}
	// setup postgres db connection
	PGresult *res;
	state->conn = PQconnectdb(state->config->db_conn_info);
	if (PQstatus(state->conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s",
				PQerrorMessage(state->conn));
		return ;
	}
	// set search_path
	res = PQexec(state->conn, "set search_path = c2v");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "set search_path failed: %s\n",
				PQerrorMessage(state->conn));
		PQclear(res);
		return ;
	}
	PQclear(res);
	fprintf(stderr, "db conn successful. \n");
	// setup notifications
	state->fd = inotify_init();

	if (state->fd < 0) {
		perror("inotify_init");
	}
	/*
	 * Need to recursively scan and add a watch to every subdirectory
	 * as well as the root.
	 * TODO: create a struct watched { int fd; char *file} and add
	 * an array of um to the state.
	 * @reload: we can either mass remove / add the watches again
	 * or only add/remove the diff from what is already in state->watches 
	 */
	state->wd = inotify_add_watch(
		state->fd, "/var/html/c2v/templates",
		IN_CLOSE_WRITE | IN_MOVE | IN_DELETE | IN_ATTRIB
	);
	// set up epoll
	state->efd = epoll_create(1);
	if (state->efd < 0) {
		perror("could not init epoll fd");
	}
	state->ev->events = EPOLLIN | EPOLLOUT | EPOLLET;
	state->cfg = epoll_ctl(state->efd, EPOLL_CTL_ADD, state->fd, state->ev);
}


static bool app_update(AppState *state)
{
	fprintf(stderr, "%s :app_update\n", get_formatted_time());
	struct inotify_event *event;
	int ret = epoll_wait(state->efd, state->ev, MAX_EVENTS, EPOLL_WAIT_MS);
	if (ret > 0) {
		int length = read(state->fd, state->buffer, BUF_LEN);
		if (length < 0) {
			perror( "read");
			return false;
		}
		for (int i=0; i < length; i++) {
			event = (struct inotify_event *)&state->buffer[i];
			if (event->len) {
				fprintf(stderr, "inotify_event len:%d mask:%d\n",
						event->len, event->mask);
			}
			// determine wtf happened.
		}
	} else if (ret < 0) {
		fprintf(stderr, "error in polling");
		return false;
	} else {
		fprintf(stderr, "poll timed out. \n");
		return true;
	}

	return true;
}

const struct AppApi app_api = {
	.create = app_create,
	.delete = app_delete,
	.unload = app_unload,
	.reload = app_reload,
	.update = app_update
};


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

#include <libpq-fe.h>

#include "app.h"
#include "core.h"
#include "walker.h"
#include "watcher.h"
#include "handlers.h"
#include "inih/ini.h"

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

static int ini_parse_handler(void *user, const char *section, const char *name, const char *value)
{
	AppConfig *config = (AppConfig *)user;
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (MATCH("", "db-address")) {
		strcpy(config->db_conn_info, value);
	} else if (MATCH("template", "root")) {
		strcpy(config->template_root, value);
	} else if (MATCH("query", "root")) {
		strcpy(config->query_root, value);
	} else {
		return 0;
	}
	return 1;
}

// App API functions.
static AppState *app_create(void)
{
	AppState *state = calloc(1, sizeof *state);
	state->config = calloc(1, sizeof *state->config);
	state->watches = calloc(1, sizeof *state->watches);
	state->buffer = calloc(1, BUF_LEN);
	state->ev = calloc(MAX_EVENTS, sizeof *state->ev);
	fprintf(stderr, "app_create(state: %p)\n", (void *)state);
	return state;
}

static void app_delete(AppState *state)
{
	fprintf(stderr, "app_delete(state: %p)\n", (void *)state);
	PQfinish(state->conn);
	free(state->config);
	free(state->buffer);
	free(state->ev);
	Watch *temp, *watch = state->watches;
	while (watch) {
		temp = watch->next;
		free(watch);
		watch = temp;
	}
	free(state);
	state = NULL;
}

static void app_unload(AppState *state)
{
	fprintf(stderr, "app_unload(state: %p)\n", (void *)state);
	// stop any notifications
	// release state->config // set conf = NULL;
	// close postgres connection
	close(state->efd); // epoll
	close(state->fd);  // inotify
	memset(state->config->db_conn_info, 0, 256);
	memset(state->config->template_root, 0, 256);
	memset(state->config->query_root, 0, 256);
	PQfinish(state->conn);
}

static void app_reload(AppState *state)
{
	fprintf(stderr, "app_reload(state: %p)\n", (void *)state);
	// load conf and set state->config
	if (ini_parse(CONF_FILE, ini_parse_handler, state->config) < 0) {
		fprintf(stderr, "error loading config file.\n");
	}
	fprintf(stderr,
			"AppConfig {db_conn_info=%s, template_root=%s, query_root=%s} @ %p \n",
			state->config->db_conn_info, state->config->template_root,
			state->config->query_root, (void *)state->config);
	// setup postgres db connection
	PGresult *res;
	fprintf(stderr, "Connecting to: %s\n", state->config->db_conn_info);
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
	// setup inotify
	add_watches(state);
	// setup epoll
	state->efd = epoll_create(1);
	if (state->efd < 0) {
		perror("could not init epoll fd");
	}
	state->ev->events = EPOLLIN | EPOLLOUT | EPOLLET;
	state->cfg = epoll_ctl(state->efd, EPOLL_CTL_ADD, state->fd, state->ev);
}

static bool app_update(AppState *state)
{
	fprintf(stderr, "%s :app_update enter\n", get_formatted_time());
	struct inotify_event *evt;
	int ret = epoll_wait(state->efd, state->ev, MAX_EVENTS, EPOLL_WAIT_MS);
	if (ret > 0) {
		int length = read(state->fd, state->buffer, BUF_LEN);
		if (length < 0) {
			fprintf(stderr, "error: read length: %d\n", length);
			fprintf(stderr, "%s :app_update exit\n", get_formatted_time());
			return true;
		}
		int i = 0;
		while (i < length ) {
			evt = (struct inotify_event *)&state->buffer[i];
			if (!evt->len) {
				fprintf(stderr, "evt->len is 0\n");
				continue;
			}
			handle_event(state, evt);
			i += (sizeof (struct inotify_event)) + evt->len;
		}
	} else if (ret < 0) {
		fprintf(stderr, "error in polling \n");
	} else {
		fprintf(stderr, "poll timed out. \n");
	}
	fprintf(stderr, "%s :app_update exit\n", get_formatted_time());
	return true;
}

const struct AppApi app_api = {
	.create = app_create,
	.delete = app_delete,
	.unload = app_unload,
	.reload = app_reload,
	.update = app_update
};


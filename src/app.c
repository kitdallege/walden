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

//#include <libpq-fe.h>

#include "app.h"
#include "core.h"
#include "config.h"
//#include "walker.h"
#include "watcher.h"
//#include "handlers.h"

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

// App API functions.
static AppState *app_create(void)
{
	AppState *self = calloc(1, sizeof *self);
	self->configurator = configurator_aloc();
	self->watcher = watcher_aloc(); 
	fprintf(stderr, "app_create(self: %p)\n", (void *)self);
	return self;
}

static void app_delete(AppState *self)
{
	fprintf(stderr, "app_delete(self: %p)\n", (void *)self);
	configurator_free(self->configurator);
	watcher_free(self->watcher);
	free(self);
	self = NULL;
}

static void app_unload(AppState *self)
{
	fprintf(stderr, "app_unload(self: %p)\n", (void *)self);
	watcher_zero(self->watcher);
	configurator_zero(self->configurator);
}

static void app_reload(AppState *self)
{
	fprintf(stderr, "app_reload(self: %p) start\n", (void *)self);
	configurator_conf(self->configurator, CONF_FILE);
	Config *conf = configurator_get_config(self->configurator);
	watcher_conf(self->watcher, conf);
	fprintf(stderr, "app_reload(self: %p) end\n", (void *)self);
	/*/ load conf and set state->config
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
	*/
}

static bool app_update(AppState *self)
{
	fprintf(stderr, "%s :app_update enter\n", get_formatted_time());
	watcher_step(self->watcher, NULL);
	return true;
}

const struct AppApi app_api = {
	.create = app_create,
	.delete = app_delete,
	.unload = app_unload,
	.reload = app_reload,
	.update = app_update
};


#ifndef APP_H
#define APP_H

#include <sys/inotify.h>
#include <libpq-fe.h>

#include "reload/reload.h"

typedef struct AppConfig
{
	char *db_conn_info;
} AppConfig;

struct AppState
{
	AppConfig *config;
	int fd, wd, efd, cfg;
	pid_t pid;
	PGconn *conn;
	char *buffer;
	struct epoll_event *ev;
};

extern const struct AppApi app_api;

#endif

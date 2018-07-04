#ifndef APP_H
#define APP_H

#include <sys/inotify.h>
#include <libpq-fe.h>

#include "reload/reload.h"

typedef struct AppConfig
{
	char db_conn_info[256];
	char template_root[256];
	char query_root[256];
} AppConfig;

typedef struct Watch
{
	int wd;
	char *path;
	struct Watch *next;
} Watch;

struct AppState
{
	AppConfig *config;
	Watch *watches;
	int fd, efd, cfg;
	pid_t pid;
	PGconn *conn;
	char *buffer;
	struct epoll_event *ev;
};

extern const struct AppApi app_api;

#endif

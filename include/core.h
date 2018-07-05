#ifndef CORE_H
#define CORE_H

//#include <sys/inotify.h>
//#include <libpq-fe.h>
#include "watcher.h"


typedef struct AppConfig
{
	char db_conn_info[256];
	char template_root[256];
	char query_root[256];
} AppConfig;

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

#endif


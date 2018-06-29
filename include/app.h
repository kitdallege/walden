#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include "reload.h"

typedef struct AppConfig
{
	char db_conn_info[512];
} AppConfig;

struct AppState
{
	AppConfig config;
};


extern const struct AppApi app_api;

#endif

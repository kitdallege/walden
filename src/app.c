#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "app.h"
#include "mem.h"

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
	AppState *state = acquire(sizeof(*state));
	return state;
}

static void app_delete(AppState *state)
{
	free(state);
	state = NULL;
}

static void app_unload(AppState *state)
{

}

static void app_reload(AppState *state)
{

}

static bool app_update(AppState *state)
{
	fprintf(stderr, "%s :app_update\n", get_formatted_time());
	return true;
}

const struct AppApi app_api = {
	.create = app_create,
	.delete = app_delete,
	.unload = app_unload,
	.reload = app_reload,
	.update = app_update
};


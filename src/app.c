#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "app.h"
#include "core.h"
#include "config.h"
#include "watcher.h"

#define CONF_FILE "./resource-mgr.conf"
#define CONN_INFO "port=5432 dbname=c2v user=c2v_admin"

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
	AppState *self		= calloc(1, sizeof *self);
	self->configurator	= configurator_aloc();
	self->watcher		= watcher_aloc(); 
	fprintf(stderr, "app_create(self: %p)\n", (void *)self);
	return self;
}

static void app_delete(AppState *self)
{
	fprintf(stderr, "app_delete(self: %p) start\n", (void *)self);
	configurator_free(self->configurator);
	watcher_free(self->watcher);
	free(self);
	self = NULL;
	fprintf(stderr, "app_delete(self: %p) end\n", (void *)self);
}

static void app_unload(AppState *self)
{
	fprintf(stderr, "app_unload(self: %p) start\n", (void *)self);
	watcher_zero(self->watcher);
	configurator_zero(self->configurator);
	fprintf(stderr, "app_unload(self: %p) end\n ", (void *)self);
}

static void app_reload(AppState *self)
{
	fprintf(stderr, "app_reload(self: %p) start\n", (void *)self);
	configurator_conf(self->configurator, CONF_FILE);
	Config *conf = configurator_get_config(self->configurator);
	watcher_conf(self->watcher, conf);
	fprintf(stderr, "app_reload(self: %p) end\n", (void *)self);
}

static bool app_update(AppState *self)
{
	fprintf(stderr, "%s :app_update enter\n", get_formatted_time());
	watcher_step(self->watcher, NULL);
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


#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "reload.h"

static long get_nanos(void);
void load_app(App *app, const char *lib, const char *api_var);
void unload_app(App *app);

Reloader *reloader_init(const char *lib, const char *api_name, unsigned int update_interval)
{
	Reloader *reloader = calloc(1, sizeof(*reloader));
	reloader->lib = lib;
	reloader->api_name = api_name;
	reloader->update_interval = update_interval;
	reloader->app = calloc(1, sizeof(*reloader->app));
	return reloader;
}

void reloader_run_loop(Reloader *reloader)
{
	App *app = reloader->app;
	bool run = true;
	long nanos, last_nanos = get_nanos();
	unsigned int update_interval = reloader->update_interval * 1000000L;
	while (run) {
		fprintf(stderr, "looping\n");
		load_app(reloader->app, reloader->lib, reloader->api_name);
		if (app->handle) {
			nanos = get_nanos();
			while (nanos - last_nanos < update_interval) {
				run = app->api.update(app->state);
				nanos = get_nanos();
			}
			last_nanos = nanos;
		}
	}
}

void reloader_shutdown(Reloader *reloader)
{
	// clean-up reloader mem.
	unload_app(reloader->app);
	free(reloader->app);
	free(reloader);
}

void load_app(App *app, const char *lib, const char *api_var)
{
	// check timestamp against current if diff < rate then NOOP.
	fprintf(stderr, "load_app:state\n");
	struct stat attr;
	if ((stat(lib, &attr) == 0) && (app->id != attr.st_ino)) {
		if (app->handle) {
			app->api.unload(app->state);
			dlclose(app->handle);
		}
		fprintf(stderr, "reloading...\n");
		void *handle = dlopen(lib, RTLD_NOW);
		if (handle) {
			app->handle = handle;
			app->id = attr.st_ino;
			const struct AppApi *api = dlsym(app->handle, api_var);	
			if (api != NULL) {
				app->api = *api;
				if (app->state == NULL) {
					app->state = app->api.create();
				}
				app->api.reload(app->state);
			} else {
				fprintf(stderr, "failed to find handle %s\n", api_var);
				dlclose(app->handle);
				app->handle = NULL;
				app->id = 0;
			}
			fprintf(stderr, "reload successful. id: %lu.\n", app->id);
		} else {
			fprintf(stderr, "failed to open lib lib %s error:%s\n", lib, dlerror());
			app->handle = NULL;
			app->id = 0;
		}
	}
	fprintf(stderr, "load_app:end\n");
}

void unload_app(App *app)
{
	if (&app->api) {
		app->api.delete(app->state);
		app->state = NULL;
	}
	if (app->handle) {
		//app->state = NULL;
		dlclose(app->handle);
		app->handle = NULL;
		app->id = 0;
	}
}

static long get_nanos(void)
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ts.tv_sec * 1000000000L + ts.tv_nsec;
}


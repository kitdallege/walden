#include <stdio.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "reload.h"


void load_app(App *app, const char *lib, const char *api_var)
{
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
				dlclose(app->handle);
				app->handle = NULL;
				app->id = 0;
			}
			fprintf(stderr, "reload successful. id: %lu.\n", app->id);
		} else {
			app->handle = NULL;
			app->id = 0;
		}
	}
}

void unload_app(App *app)
{
	if (app->handle) {
		app->state = NULL;
		dlclose(app->handle);
		app->handle = NULL;
		app->id = 0;
	}
}


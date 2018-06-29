#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "reload.h"
#include "app.h"
#include "mem.h"

const char *lib = "/srv/code/resource-mgr/build/libresource-mgr.so";
const char *api_var = "app_api";

int main(int argc, char **argv)
{
	/*
	 * All main does is load the app shared-lib  and call its update
	 * function until it returns bool:false.
	 * It also watches for changes/updates to the app lib, and reloads
	 * the app automatically when that occurres.
	 */
	memory_init();
	struct App app = {0};
	bool run = true;
	while (run) {
		load_app(&app, lib, api_var);
		if (app.handle) {
			run = app.api.update(app.state);
		}
		usleep(1000000);
	}
	unload_app(&app);
	memory_free();
}


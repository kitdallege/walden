#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

#include "reload.h"
#include "app.h"
#include "mem.h"

struct App app = {0};
const char *lib = "./build/libresource-mgr.so";
const char *api_var = "app_api";

static void handle_signal(int signal)
{
	
	unload_app(&app);
	mem_system_release();
	//pthread_exit(NULL);
	// cleans up some thread local (makes valgrind happy) 
	// https://stackoverflow.com/a/1874334
	// sadly it also seems to cause segfaults so. meh. handy for valgrind
	// testing but not much else...
	// basically dlopen allocs memory for errormsg in a thread_local storage.
	// whilst its cleaned up automaticall at process exit, it still 
	// pisses valgrind off. ;P
	(void) mem_write_report(stderr);
	exit(1);
}

int main(int argc, char **argv)
{
	/*
	 * main loads the app's shared-lib and call its update function until
	 * it returns bool:false.
	 * It also watches for changes/updates to the app lib, and reloads
	 * it automatically.
	 */
	signal(SIGINT, handle_signal); 
	memory_init();
	bool run = true;
	while (run) {
		load_app(&app, lib, api_var); // add a rate msec arg & remove usleep
		if (app.handle) {
			run = app.api.update(app.state);
		}
		usleep(1000000);
	}
	unload_app(&app);
	mem_system_release();
}


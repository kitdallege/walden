#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

#include "reload/reload.h"

Reloader *reloader;
const char *lib = "./build/libresource-mgr.so";
const char *api_var = "app_api";
unsigned int check_rate = 5000; // check every 5 seconds.

static void handle_signal(int signal)
{
	
	reloader_shutdown(reloader);
	//pthread_exit(NULL);
	// cleans up some thread locals (making valgrind happy) 
	// https://stackoverflow.com/a/1874334
	// sadly it also seems to cause segfaults every so often.. 
	// so. meh. for that idea.. its still handy for valgrind testing but
	// not much else...
	// dlopen allocs memory for errormsg in a thread_local storage.
	// whilst its cleaned up automaticall at process exit, it still 
	// pisses valgrind off, and thus ya hear about 32 bytes still reachable.
	exit(1);
}

int main(int argc, char **argv)
{
	// see: app.c for application code
	reloader = reloader_init(lib, api_var, check_rate);
	signal(SIGINT, handle_signal); 
	reloader_run_loop(reloader);
	reloader_shutdown(reloader);
	exit(0);
}


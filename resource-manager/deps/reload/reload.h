#ifndef RELOAD_H
#define RELOAD_H

#include <stdbool.h>
#include <sys/stat.h>

// AppState struct is defined by the 'host' application.
typedef struct AppState AppState;

struct AppApi
{
	AppState *(*create)(void);
	void      (*delete)(AppState *state);
	void      (*unload)(AppState *state);
	void      (*reload)(AppState *state);
	bool      (*update)(AppState *state);
};

typedef struct App
{
	AppState *state;
	void *handle;
	ino_t id;
	struct AppApi api;
} App;

typedef struct Reloader
{
	const char *lib;
	const char *api_name;
	unsigned int update_interval;
	App *app;
} Reloader;

// Public API
Reloader *reloader_init(const char *lib, const char *api_name, unsigned int update_interval);
void reloader_run_loop(Reloader *reloader);
void reloader_shutdown(Reloader *reloader);

#endif

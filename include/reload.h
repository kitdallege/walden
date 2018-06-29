#ifndef RELOAD_H
#define RELOAD_H
#include <stdbool.h>
#include <sys/stat.h>

typedef struct AppState AppState;

struct AppApi
{
	AppState *(*create)(void);
	void (*delete)(AppState *state);
	void (*unload)(AppState *state);
	void (*reload)(AppState *state);
	bool (*update)(AppState *state);
};

typedef struct App
{
	void *handle;
	ino_t id;
	struct AppApi api;
	AppState *state;
} App;

void load_app(App *app, const char *lib, const char *api_var);
void unload_app(App *app);

#endif

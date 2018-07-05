#ifndef WATCHER_H
#define WATCHER_H

typedef struct AppState AppState;

typedef struct Watch
{
	int wd;
	char *path;
	struct Watch *next;
} Watch;

void add_watches(AppState *state);

#endif

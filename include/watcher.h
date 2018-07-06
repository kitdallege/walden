#ifndef WATCHER_H
#define WATCHER_H

typedef struct Handler Handler;				// defined in handlers.h
typedef struct Watcher Watcher;

// life cycle
Watcher    *watcher_aloc(void);
void		watcher_conf(Watcher *self, void *user);
void		watcher_zero(Watcher *self);
void		watcher_free(Watcher *self);

// api 
void watcher_step(Watcher *self, void *user);

// tech private api (may go internal/static)
void watcher_add_watch(Watcher *self, const char *path);
void watcher_remove_watch(Watcher *self, const char *path);

#endif

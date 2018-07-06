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

// this fits the step model pretty damn well. 0.o 
void watcher_step(Watcher *self, void *user);
void watcher_process_events(Watcher *self);

// tech private api ?
void watcher_add_watch(Watcher *self, const char *path);
void watcher_remove_watches(Watcher *self);

// TODO: accept Watcher * as arg.
// if 'more info' is needed from AppState pass in as vars to functions.
//void watcher_add_watches(Watcher *self);
//void watcher_remove_watches(Watcher *self);
// TODO: rename to update
//void watcher_process_events(Watcher *self);

#endif

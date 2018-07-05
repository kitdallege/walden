#ifndef HANDLERS_H
#define HANDLERS_H

typedef struct AppState AppState;
typedef struct inotify_event InotifyEvent;

void handle_event(AppState *state, InotifyEvent *evt);

#endif

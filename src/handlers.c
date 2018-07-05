#include <stdlib.h>
#include <stdio.h>

#include <sys/inotify.h>
#include <libpq-fe.h>

#include "handlers.h"
#include "core.h"

static void add_watch(AppState *state, char *dirname);

void handle_event(AppState *state, InotifyEvent *evt)
{
	int mask = evt->mask & (IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED);
	fprintf(
		stderr,
		"inotify_event wd:%d mask:%d cookie:%d len:%d name:%s \n",
		evt->wd, evt->mask, evt->cookie, evt->len, evt->name
	);
	fprintf(stderr, "switch value: %#010x \n", mask);
	switch (mask) {
		case IN_ISDIR: {
			// add watch
			fprintf(stderr, "IN_ISDIR\n");
			add_watch(state, evt->name);
			break;
		}
		case IN_CLOSE_WRITE:
			fprintf(stderr, "IN_CLOSE_WRITE\n");
			break;
		case IN_CREATE:
			fprintf(stderr, "IN_CREATE \n");
			break;
		case IN_DELETE:
			fprintf(stderr, "IN_DELETE \n");
		default:
			fprintf(stderr, "unhandled event type\n");
			break;
	}
}

static void add_watch(AppState *state, char *dirname)
{
	// TODO: check for existing watch for dir ?
	int wd = inotify_add_watch(
		state->fd, dirname,
		IN_CLOSE_WRITE | IN_MOVE | IN_DELETE | IN_ATTRIB
	);
	Watch *watch = malloc(sizeof *watch);
	*watch = (Watch){.wd=wd, .path=dirname};
	watch->next = state->watches;
	state->watches = watch;
}




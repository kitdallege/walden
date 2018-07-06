#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/inotify.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>

#include <libpq-fe.h> // because of app state.

#include "hash_table/hash_table.h"
#include "hash_table/fnv_hash.h"

#include "watcher.h"
#include "handlers.h"
#include "config.h"
#include "walker.h"

#define EPOLL_WAIT_MS 1000 
#define MAX_EVENTS 64
#define WATCH_MASK (IN_CLOSE_WRITE | IN_MOVE | IN_DELETE | IN_ATTRIB)
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN    (1024 * (EVENT_SIZE + 16))

typedef struct hash_table HashTable;

// state
struct Watcher
{
	int cfg, fd, efd;
	struct epoll_event *ev;
	char *buffer;
	HashTable *watches;
	Handler *handler;
};

static char* get_formatted_time(void)
{
    time_t rawtime;
    struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Must be static, otherwise won't work
    static char _retval[20];
    strftime(_retval, sizeof(_retval), "%Y-%m-%d %H:%M:%S", timeinfo);

    return _retval;
}
// life cycle 
Watcher *watcher_aloc()
{
	Watcher *self	= calloc(1, sizeof *self);
	self->watches	= hash_table_create_for_string();
	self->buffer	= calloc(1, BUF_LEN);
	self->ev		= calloc(MAX_EVENTS, sizeof *self->ev);
	self->handler	= handler_aloc();
	return self;
}

void watcher_conf(Watcher *self, void *user)
{
	Config *conf = (Config *)user;
	handler_conf(self->handler, conf->db_conn_info);
	// setup notifications
	self->fd = inotify_init();
	if (self->fd < 0) {
		fprintf(stderr, "inotify_init \n");
	}
	self->efd = epoll_create(1);
	if (self->efd < 0) {
		fprintf(stderr, "could not init epoll fd in  %s:%d\n", __FILE__, __LINE__);
	}
	self->ev->events = EPOLLIN | EPOLLOUT | EPOLLET;
	self->cfg = epoll_ctl(self->efd, EPOLL_CTL_ADD, self->fd, self->ev);
	watcher_add_watch(self, conf->template_root);
	watcher_add_watch(self, conf->query_root);
}

void watcher_step(Watcher *self, void *user)
{
	struct inotify_event *evt;
	int ret = epoll_wait(self->efd, self->ev, MAX_EVENTS, EPOLL_WAIT_MS);
	if (ret > 0) {
		int length = read(self->fd, self->buffer, BUF_LEN);
		if (length < 0) {
			fprintf(stderr, "error: read length: %d\n", length);
			fprintf(stderr, "%s :app_update exit\n", get_formatted_time());
			return ;
		}
		int i = 0;
		while (i < length ) {
			evt = (struct inotify_event *)&self->buffer[i];
			if (!evt->len) {
				fprintf(stderr, "evt->len is 0\n");
				continue;
			}
			handler_step(self->handler, evt); 
			i += (sizeof (struct inotify_event)) + evt->len;
		}
	} else if (ret < 0) {
		fprintf(stderr, "error in polling \n");
	} else {
		fprintf(stderr, "poll timed out. \n");
	}
}

void watcher_zero(Watcher *self)
{
	// memset stuffs
	handler_zero(self->handler);
	watcher_remove_watches(self);
}

void watcher_free(Watcher *self)
{
	hash_table_destroy(self->watches, NULL);
	free(self->buffer);
	free(self->ev);	
	handler_free(self->handler);
	free(self);
}

void watcher_add_watch(Watcher *self, const char *path)
{
	Dirs *dirs = find_dirs(path);
	for (int i=0, dirs_len = dirs->count ; i < dirs_len; i++) {
		int *wd = malloc(sizeof(*wd));
		*wd = inotify_add_watch(self->fd, dirs->paths[i], WATCH_MASK);
		hash_table_insert(self->watches, dirs->paths[i], wd);
		fprintf(stderr, "adding watch: %s -> %d\n", dirs->paths[i], *wd);
	}
	free_dirs(dirs);
}

void watcher_remove_watches(Watcher *self)
{
	struct hash_entry *entry;
	for (entry = hash_table_next_entry(self->watches, NULL); entry != NULL;
		 entry = hash_table_next_entry(self->watches, entry)) {
		inotify_rm_watch(self->fd, *((int *)entry->data));	
		free(entry->data);
		hash_table_remove_entry(self->watches, entry);
	}
}


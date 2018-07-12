#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

const char *watcher_get_watch_path_by_fid(Watcher *self, int wd);
// state
struct Watcher
{
	int cfg, fd, efd;
	struct epoll_event *ev;
	char *buffer;
	HashTable *watches;
	Handler *handler;
};

typedef struct Watch
{
	int wd;
	char *path;
} Watch;

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
static uint32_t key_value(const void *key)
{
	return *(const uint32_t *)key;
}

static int uint32_t_key_equals(const void *a, const void *b)
{
	return key_value(a) == key_value(b);
}

// life cycle 
Watcher *watcher_aloc()
{
	Watcher *self	= calloc(1, sizeof *self);
	self->watches	= hash_table_create(key_value, uint32_t_key_equals);
	self->buffer	= calloc(1, BUF_LEN);
	self->ev		= calloc(MAX_EVENTS, sizeof *self->ev);
	self->handler	= handler_aloc();
	return self;
}

void watcher_conf(Watcher *self, void *user)
{
	Config *conf = (Config *)user;
	handler_conf(self->handler, conf);
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
	handler_sync_all(self->handler);
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
			i += (sizeof (struct inotify_event)) + evt->len;
			if (!evt->len) {
				fprintf(stderr, "evt->len is 0\n");
				continue;
			}
			// check event.mask isdir. if so. add/remove its associated watch.
			if (evt->mask & IN_ISDIR) {
				watcher_add_watch(self, evt->name);
				continue;
			}
			// handle file events
			FileEvent *fevent = calloc(1, sizeof *fevent);
			//fevent->filename = evt->name;
			const char *path = watcher_get_watch_path_by_fid(self, evt->wd);
			fprintf(stderr, "watcher_get_watch_path_by_fid: %s\n", path);
			char *fullpath = calloc(1, sizeof *fullpath * (strlen(path) + strlen(evt->name) + 2));
			sprintf(fullpath, "%s/%s", path, evt->name); 
			fullpath[strlen(path) + strlen(evt->name) + 1] = '\0';
			fevent->filename = fullpath; // its up to fevent to free fullpath
			int mask = evt->mask & (
				IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED
			);
			switch (mask) {
				case IN_CREATE:
					fprintf(stderr, "IN_CREATE \n");
					fevent->type = FET_ADD;
					break;
				case IN_CLOSE_WRITE:
					fprintf(stderr, "IN_CLOSE_WRITE\n");
					fevent->type = FET_MOD;
					break;
				case IN_DELETE:
					fprintf(stderr, "IN_DELETE \n");
					fevent->type = FET_DEL;
				default:
					fprintf(stderr, "unhandled event type\n");
					break;
			}
			handler_enqueue_event(self->handler, fevent); 
		}
	} else if (ret < 0) {
		fprintf(stderr, "error in polling \n");
	} else {
		//fprintf(stderr, "poll timed out. \n");
	}
	// [to reduce re-renders] the handler baches changes using a buffer. 
	// once no activity is seen for X amount  of time, it updates the db
	// and notifies the renderer of pending work.
	handler_step(self->handler, NULL);
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
		Watch *watch = calloc(1, sizeof *watch);
		watch->wd = inotify_add_watch(self->fd, dirs->paths[i], WATCH_MASK);
		watch->path = strdup(dirs->paths[i]);
		hash_table_insert(self->watches, &watch->wd, watch);
		fprintf(stderr, "adding watch: %s -> %d\n", watch->path, watch->wd);
	}
	free_dirs(dirs);
}

void watcher_remove_watches(Watcher *self)
{
	struct hash_entry *entry;
	for (entry = hash_table_next_entry(self->watches, NULL); entry != NULL;
		 entry = hash_table_next_entry(self->watches, entry)) {
		inotify_rm_watch(self->fd, *((int *)entry->data));
		free(((Watch *)entry->data)->path);
		free(entry->data);
		hash_table_remove_entry(self->watches, entry);
	}
}
// TODO: this is O(n), which sucks. need to store the map 
// the other way around, int -> char * as we only care about
// lookups from int -> char. This only 'doesn't matter' atm
// because N is fairly small. 
const char *watcher_get_watch_path_by_fid(Watcher *self, int wd)
{

	fprintf(stderr, "watcher_get_watch_path_by_fid: wd: %d \n", wd);
	const char *result;
	struct hash_entry *entry;
	for (entry = hash_table_next_entry(self->watches, NULL); entry != NULL;
		 entry = hash_table_next_entry(self->watches, entry)) {
		fprintf(stderr, "watcher_get_watch_path_by_fid: key: %d \n",
				((Watch *)entry->data)->wd);
		if (((Watch *)entry->data)->wd == wd) {
			result = ((Watch *)entry->data)->path;
			break;
		}
	}
	fprintf(stderr, "watcher_get_watch_path_by_fid: result: %s\n", result);
	return result;
}


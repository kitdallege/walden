#include <stdlib.h>
#include <sys/inotify.h>

#include <libpq-fe.h> // because of app state.

#include "watcher.h"
#include "walker.h"
#include "core.h"

/*
 * Need to recursively scan and add a watch to every subdirectory
 * as well as the root.
 * TODO: create a struct watched { int fd; char *file} and add
 * an array of um to the state.
 * @reload: we can either mass remove / add the watches again
 * or only add/remove the diff from what is already in state->watches 
 */
void add_watches(AppState *state)
{
	// add watches for template dirs.
	Dirs *dirs = find_dirs(state->config->template_root);
	int template_dirs_len = dirs->count;
	for (int i=0; i < template_dirs_len; i++) {
		int wd = inotify_add_watch(
			state->fd, dirs->paths[i],
			IN_CLOSE_WRITE | IN_MOVE | IN_DELETE | IN_ATTRIB
		);
		Watch *watch = malloc(sizeof *watch);
		*watch = (Watch){.wd=wd, .path=dirs->paths[i]};
		watch->next = state->watches;
		state->watches = watch;
	}
	free_dirs(dirs);
	dirs = find_dirs(state->config->query_root);
	for (unsigned int i=0; i < dirs->count; i++) {
		int wd = inotify_add_watch(
			state->fd, dirs->paths[i],
			IN_CLOSE_WRITE | IN_MOVE | IN_DELETE | IN_ATTRIB
		);
		Watch *watch = malloc(sizeof *watch);
		*watch = (Watch){.wd=wd, .path=dirs->paths[i]};
		watch->next = state->watches;
		state->watches = watch;
	}
	free_dirs(dirs);
}

/*
 * Program which renders a webpage when told via postgres (listen/notify).
 *
 * Listens for 'dirty_webpage' and parses the json payload it contains
 * into a struct.
 *
 * Combines template & query to produce a page, also writes a pjax file
 * "which is the contents of <main></main> served under /_/path/to/page.html".
 *
 * Configs:
 *   - root_dir:      Root path which the others reside within.
 *   - template_dir
 *   - web_dir
 *   - query_dir
 * 
 */
#include <stdio.h>
#include <signal.h>

#include "renderer.h"

static RendererState *renderer_state;

static void handle_signal(int signal)
{
	fprintf(stderr, "\n Caught Signal: %d \n", signal);
	renderer_api.delete(renderer_state);
	exit(1);
	/*
	fprintf(stderr, "size: %lu, active: %d : ",
			bqueue_size(renderer_state->flipper->wq),
			renderer_state->flipper->ctl->active);
	pthread_cond_signal(&(renderer_state->flipper->ctl->cond));
	*/
}

int main(int argc, char **argv)
{
	// create render
	signal(SIGINT, handle_signal); 
	bool run = true;
	renderer_state = renderer_api.create();
	if (!renderer_state) {
		fprintf(stderr, "init failed!\n");
		return 1;
	}
	while (run) {
		run = renderer_api.update(renderer_state);
	}
	renderer_api.delete(renderer_state);
	return 0;
}


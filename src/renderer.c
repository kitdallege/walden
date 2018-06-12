#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <pthread.h>

#include "templates.h"
#include "page_spec.h"
#include "files.h"
#include "query.h"
#include "renderer.h"

static char root_dir[] = "/var/html/c2v";
static char web_dir[] = "www";

int handle_page(PGconn *conn, FlagFlipperState *flipper, const char *payload)
{
	// parse payload into a struct
	PageSpec *spec = parse_page_spec(payload);
	if (!spec) {
		fprintf(stderr, "unable to parse page_spec\n");
		return 0;
	}
	// check template & query exist
	if (!file_exists(spec->template)) {
		fprintf(stderr, "missing template: %s\n", spec->template);
		free_page_spec(spec);
		return 1;
	};
	if (!file_exists(spec->query)) {
		fprintf(stderr, "missing query: %s\n", spec->query);
		free_page_spec(spec);
		return 1;
	};
	// run query and get text response 
	char *json_data = get_query_result(conn, spec->query, spec->query_params);
	if (!json_data) {
		fprintf(stderr, "query returned no data:%s\n", spec->query);
		free_page_spec(spec);
		return 1;
	}
	// render template and get html text
	char *html = render_template(spec->template, json_data);
	if (!html) {
		fprintf(stderr, "render_template had zero length result.\n");
		free_page_spec(spec);
		free(json_data);
		return 1;
	}
	// write html to disk
	if (write_page(spec->filename, spec->path, html)) {
		fprintf(stderr, "unable to write html file.\n");
		free_page_spec(spec);
		free(json_data);
		free(html);
		return 1;
	}
	// write pjax (<main> block of html) to '_' directory
	if (write_pjax(spec->filename, spec->path, html)) {
		fprintf(stderr, "unable to write pjax file.\n");
		free_page_spec(spec);
		free(json_data);
		free(html);
		return 1;
	}
	// update webpage setting dirty flag to false.
	// TODO: this is a major bottle neck as well. and needs to be 
	// done (in the background (and/or) in batch).
	// we should be somewhere around 350mb/sec if were sync'n
	// and up to multiple gigs a second if were not. so this can def
	// improve. (note: the 1tb spinning hdd is about 120 MB/s)
	// might be good for a test target.
	// without sync in write_file & this call were @ 18 MB/s so 
	// in theory we could 20x that before the disk was stopping us.
	// that would put us @ 20-40 µs (microseconds) per page
	// or roughly 25K pages per second.
	// realistically 5K a secound (200 micros) per page is a realistic goal.
	// which has us at about 180 MB/s write speed.
	/*
	if (webpage_clear_dirty_flag(conn, spec->id)) {
		fprintf(stderr, "unable to clear dirty flag: page{id=%d}\n",
				spec->id);
		free_page_spec(spec);
		free(json_data);
		free(html);
		return 1;
	} */ 
	// clear_dirty_flag(spec->id);
	// add to a queue so that the background thread can process in chunks.
	pthread_mutex_lock(&(flipper->ctl.mutex));
	bqueue_push(flipper->wq, &spec->id);
	pthread_mutex_unlock(&(flipper->ctl.mutex));
	//pthread_cond_broadcast(&(flipper->ctl.cond));
	// cleanup
	free_page_spec(spec);
	free(json_data);
	free(html);
	return 0;
}

int write_page(const char *name, const char *path, const char *data)
{
	//fprintf(stderr, "write_page: name:\"%s\" path:\"%s\"\n", name, path);
	int ret = 0;
	char *filename = mk_abs_path(root_dir, web_dir, (char *)path,
					(char *)name, NULL);
	char *dir = strdup(filename);
	if (mkdir_p(dirname(dir))) {
		fprintf(stderr, "error making directory: \"%s\"\n", dir);
		ret =  -1;
	}
	if(write_file(filename, data)) {
		fprintf(stderr, "write_page failed.\n");
		ret = -1;
	}
	free(filename);
	free(dir);
	return ret;
}

int write_pjax(const char *name, const char *path, const char *data)
{
	//fprintf(stderr, "write_pjax: name:\"%s\" path:\"%s\"\n", name, path);
	int ret = 0;
	char pjax_dir[] = "_";
	char *filename = mk_abs_path(root_dir, web_dir, pjax_dir,
					(char *)path, (char *)name, NULL);
	char *dir = strdup(filename);
	if (mkdir_p(dirname(dir))) {
		fprintf(stderr, "error making directory: \"%s\"\n", dir);
		ret = -1;
	}
	// parse out main from data
	char *start = strstr(data, "<main>");
	char *end = strstr(start, "</main>");
	char *pdata  = calloc(strlen(data) + 1, sizeof(*pdata));
	strncpy(pdata, start, (end + 7) - start);
	if (write_file(filename, pdata)) {
		fprintf(stderr, "write_pjax failed.\n");
		ret = -1;
	}
	free(filename);
	free(dir);
	free(pdata);
	return ret;
}


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

typedef struct timespec timespec;

static timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}


int handle_page(PGconn *conn, FlagFlipperState *flipper, const char *payload)
{
	timespec ct1, ct2, pt1, pt2, td1, td2;
	clock_gettime(CLOCK_MONOTONIC, &ct1); clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt1);
	// parse payload into a struct
	PageSpec *spec = parse_page_spec(payload);
	if (!spec) {
		fprintf(stderr, "unable to parse page_spec\n");
		return 0;
	}
	clock_gettime(CLOCK_MONOTONIC, &ct2); clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt2);
	td1 = diff(ct1, ct2); td2 = diff(pt1, pt2);
	fprintf(stderr, "\tparse_page_spec: system time elapsed: %.3f msec / %ld ns cpu time elapsed: %.3f msec / %ld ns\n ", td1.tv_nsec / 1000000.0, td1.tv_nsec, td2.tv_nsec / 1000000.0, td2.tv_nsec); 
	clock_gettime(CLOCK_MONOTONIC, &ct1); clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt1);
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
	clock_gettime(CLOCK_MONOTONIC, &ct2); clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt2);
	td1 = diff(ct1, ct2); td2 = diff(pt1, pt2);
	fprintf(stderr, "\tfile_exists: system time elapsed: %.3f msec / %ld ns cpu time elapsed: %.3f msec / %ld ns\n ", td1.tv_nsec / 1000000.0, td1.tv_nsec, td2.tv_nsec / 1000000.0, td2.tv_nsec); 
	clock_gettime(CLOCK_MONOTONIC, &ct1); clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt1);
	// run query and get text response 
	char *json_data = get_query_result(conn, spec->query, spec->query_params);
	if (!json_data) {
		fprintf(stderr, "query returned no data:%s\n", spec->query);
		free_page_spec(spec);
		return 1;
	}
	clock_gettime(CLOCK_MONOTONIC, &ct2); clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt2);
	td1 = diff(ct1, ct2); td2 = diff(pt1, pt2);
	fprintf(stderr, "\tget_query_result: system time elapsed: %.3f msec / %ld ns cpu time elapsed: %.3f msec / %ld ns\n ", td1.tv_nsec / 1000000.0, td1.tv_nsec, td2.tv_nsec / 1000000.0, td2.tv_nsec); 
	clock_gettime(CLOCK_MONOTONIC, &ct1); clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt1);
	// render template and get html text
	char *html = render_template(spec->template, json_data);
	if (!html) {
		fprintf(stderr, "render_template had zero length result.\n");
		free_page_spec(spec);
		free(json_data);
		return 1;
	}
	clock_gettime(CLOCK_MONOTONIC, &ct2); clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt2);
	td1 = diff(ct1, ct2); td2 = diff(pt1, pt2);
	fprintf(stderr, "\trender_template: system time elapsed: %.3f msec / %ld ns cpu time elapsed: %.3f msec / %ld ns\n ", td1.tv_nsec / 1000000.0, td1.tv_nsec, td2.tv_nsec / 1000000.0, td2.tv_nsec); 
	clock_gettime(CLOCK_MONOTONIC, &ct1); clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt1);
	// write html to disk
	if (write_page(spec->filename, spec->path, html)) {
		fprintf(stderr, "unable to write html file.\n");
		free_page_spec(spec);
		free(json_data);
		free(html);
		return 1;
	}
	clock_gettime(CLOCK_MONOTONIC, &ct2); clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt2);
	td1 = diff(ct1, ct2); td2 = diff(pt1, pt2);
	fprintf(stderr, "\twrite_page: system time elapsed: %.3f msec / %ld ns cpu time elapsed: %.3f msec / %ld ns\n ", td1.tv_nsec / 1000000.0, td1.tv_nsec, td2.tv_nsec / 1000000.0, td2.tv_nsec); 
	clock_gettime(CLOCK_MONOTONIC, &ct1); clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt1);
	// write pjax (<main> block of html) to '_' directory
	if (write_pjax(spec->filename, spec->path, html)) {
		fprintf(stderr, "unable to write pjax file.\n");
		free_page_spec(spec);
		free(json_data);
		free(html);
		return 1;
	}
	clock_gettime(CLOCK_MONOTONIC, &ct2); clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt2);
	td1 = diff(ct1, ct2); td2 = diff(pt1, pt2);
	fprintf(stderr, "\twrite_pjax: system time elapsed: %.3f msec / %ld ns cpu time elapsed: %.3f msec / %ld ns\n ", td1.tv_nsec / 1000000.0, td1.tv_nsec, td2.tv_nsec / 1000000.0, td2.tv_nsec); 
	clock_gettime(CLOCK_MONOTONIC, &ct1); clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt1);
	// clear_dirty_flag: add to a queue for a background thread to process.
	pthread_mutex_lock(&(flipper->ctl->mutex));
	unsigned int *id = malloc(sizeof(*id));
	*id = spec->id;
	bqueue_push(flipper->wq, id);
	pthread_mutex_unlock(&(flipper->ctl->mutex));

	// cleanup
	free_page_spec(spec);
	free(json_data);
	free(html);
	clock_gettime(CLOCK_MONOTONIC, &ct2); clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt2);
	td1 = diff(ct1, ct2); td2 = diff(pt1, pt2);
	fprintf(stderr, "\tcleanup: system time elapsed: %.3f msec / %ld ns cpu time elapsed: %.3f msec / %ld ns\n ", td1.tv_nsec / 1000000.0, td1.tv_nsec, td2.tv_nsec / 1000000.0, td2.tv_nsec); 
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


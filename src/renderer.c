#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <pthread.h>

#include "deps/hash_table/hash_table.h"
#include "deps/hash_table/fnv_hash.h"

#include <json-c/json.h> 

#include "log.h"
#include "templates.h"
#include "page_spec.h"
#include "files.h"
#include "flag_flipper.h"
#include "query.h"
#include "renderer.h"

static char root_dir[] = "/var/html/c2v";
static char template_dir[] = "templates";
static char web_dir[] = "www";
static char query_dir[] = "queries";
/*
#define GET_DIRTY_SQL  "
select
	p.id,
	p.name || '.html' as filename, 
	"replace(p.parent_path::text, '.', '/') as path, 
	"spec.template,
	spec.query,
	p.query_params "\
 */
static bool is_scalar(int page_spec_id)
{
	bool ret = false;
	switch (page_spec_id) {
		case 1: {
		   	ret = true;
			break;
		}
		default: {
			ret = false;
			break;
		 }
	}
	return ret;
}

static int handle_pages_scalar(PGconn *conn, FlagFlipperState *flipper,
		PGresult *results, int results_len, int spec_id, char *template_data,
		char *query_file, json_object *global_context)
{
	fprintf(stderr, "START handle_pages_scalar \n");
	// query per page.
	PGresult *res;
	PageIdArray *page_ids = page_id_array_create(results_len);

	for (int i = 0; i < results_len; i++) {
		page_ids->data[i] = atoi(PQgetvalue(results, i, 0));
		page_ids->len++;
		char *cmd = strdup(query_file); // TODO: reuse a buffer instead of dup'n
		char *params = PQgetvalue(results, i, 5);
		rewrite_query(&cmd, params);
		res = PQexec(conn, cmd);
		if (PQresultStatus(res) != PGRES_TUPLES_OK) {
			fprintf(stderr, "get_query_result failed: cmd: %s  %s\n", cmd, PQerrorMessage(conn));
			PQclear(res);
			free(cmd);
			fprintf(stderr, "END handle_pages_scalar \n");
			return 1;
		}
		free(cmd);
		// build context
		char *context_data = PQgetvalue(res, 0, 0);
		json_object *obj = json_tokener_parse(context_data);
		json_object_object_add(obj, "CWD", json_object_new_string("/var/html/c2v/templates"));
		json_object_object_merge(obj, global_context);
		// render
		char *html = render_template_str(template_data, obj);
		//write files
		const char *filename = PQgetvalue(results, i, 1);
		char *path = PQgetvalue(results, i, 2);
		path += 5; // walk path forward past the first dir.
		if (write_page(filename, path, html)) {
			fprintf(stderr, "unable to write html file. filename:%s path:%s\n", filename, path);
		}
		if (write_pjax(filename, path, html)) {
			fprintf(stderr, "unable to write pjax file. filename:%s path:%s\n", filename, path);
		}
		free(html);
		json_object_put(obj);
		PQclear(res);
	}
	pthread_mutex_lock(&(flipper->ctl->mutex));
	// TODO: use a struct so we can store len along side the ids
	bqueue_push(flipper->wq, page_ids);
	pthread_mutex_unlock(&(flipper->ctl->mutex));
	fprintf(stderr, "END handle_pages_scalar \n");
	return 0;
}

int augment_query(char **query_file, char **query_params, int params_len);
int augment_query(char **query_file, char **query_params, int params_len)
{
	int param_len = 0;
	size_t temp_size = sizeof(char *) * (params_len * 40);
	char *bp,  *temp;
    bp = temp = malloc(temp_size);
	// TODO: figure out why garbage memory is blowing this up..
	// would be nice to nix the memset.
	memset(bp, 0, temp_size);
	strcpy(temp, " WHERE pk = ANY('{");
	temp += 18;
	for (int i = 0; i < params_len; i++) {
		char *str = query_params[i];
		param_len = strlen(str);
		if (i) { *temp++ = ',';}
		strcat(temp, query_params[i]);
		temp +=  param_len;
	}
	strcpy(temp, "}')");
	temp += 3;
	*temp = '\0';
	//fprintf(stderr, "in augment: bp: %s\n", bp);
	*query_file = realloc(*query_file, strlen(*query_file) + temp_size + 4);
	// need to rip off everything to & including ';'
	// add where clause with ;
	char *semi = *query_file + strlen(*query_file) - 1;
	for(;*semi != ';'; semi--){};	
	*semi = '\0';
	strcat(*query_file, bp);
	strcat(*query_file, ";");
	free(bp);
	//fprintf(stderr, "in augment: query_file: %s\n", *query_file);
	return 0;
}



static int handle_pages_vector(PGconn *conn, FlagFlipperState *flipper,
		PGresult *results, int results_len, int spec_id, char *template_data,
		char **query_file, json_object *global_context)
{
	fprintf(stderr, "START handle_pages_vector \n");
	if (!results_len) { return 0; }
	// 1 query for all pages.
	PGresult *res;
	char *query_param;
	char *query_params[results_len];
	int row_ids[results_len];
	//int page_ids[results_len];
	PageIdArray *page_ids = page_id_array_create(results_len);
	struct hash_table *ptable;
	ptable = hash_table_create_for_string();
	for (int i = 0; i < results_len; i++) {
		query_param = PQgetvalue(results, i, 5);	
		query_params[i] = query_param;
		row_ids[i] = i;
		hash_table_insert(ptable, query_param, &row_ids[i]);
		unsigned int pk = atoi(PQgetvalue(results, i, 0));
		page_ids->data[i] = pk; page_ids->len++;
	}
	//fprintf(stderr, "query_file pre agument: %s\n", *query_file);
	augment_query(query_file, query_params, results_len);
	//fprintf(stderr, "query_file post agument: %s\n", *query_file);
	res = PQexec(conn, *query_file);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "error with query: %s error:%s\n", *query_file,
				PQerrorMessage(conn));
		fprintf(stderr, "END handle_pages_vector \n");
		return 0;
	}
	//fprintf(stderr, "vectorized query returned %d rows. \n", PQntuples(res));
	if (!PQntuples(res)) {
		fprintf(stderr, "END handle_pages_vector \n");
		return 0;
	}
	struct hash_entry *item;	
	// check: len(res) == len(results)
	for (int i = 0; i < results_len; i++) {
		// create context
		char *json_data = PQgetvalue(res, i, 1);
		json_object *obj = json_tokener_parse(json_data);
		json_object_object_add(obj, "CWD", json_object_new_string("/var/html/c2v/templates"));
		json_object_object_merge(obj, global_context);
		// render
		char *html = render_template_str(template_data, obj);
		// write html files
		item = NULL;
		item = hash_table_search(ptable, PQgetvalue(res, i, 0));
		if (!item) {
			fprintf(stderr, "unable to find item for: %s\n", PQgetvalue(res, i, 0));
		}
		int row_idx = *(int *)item->data; 
		const char *filename = PQgetvalue(results, row_idx, 1);
		char *path = PQgetvalue(results, row_idx, 2);
		path += 5; // walk path forward past the first dir.
		if (write_page(filename, path, html)) {
			fprintf(stderr, "unable to write html file. filename:%s path:%s\n", filename, path);
		}
		if (write_pjax(filename, path, html)) {
			fprintf(stderr, "unable to write pjax file. filename:%s path:%s\n", filename, path);
		}
		free(html);
		json_object_put(obj);
	}
	hash_table_destroy(ptable, NULL);
	PQclear(res);
	pthread_mutex_lock(&(flipper->ctl->mutex));
	bqueue_push(flipper->wq, page_ids);
	pthread_cond_broadcast(&(flipper->ctl->cond));
	pthread_mutex_unlock(&(flipper->ctl->mutex));
	fprintf(stderr, "END handle_pages_vector \n");
	return 0;
}

int handle_pages(PGconn *conn, FlagFlipperState *flipper, PGresult *results,
		int spec_id, json_object *global_context)
{
	// load up template_file
	char *template_path = mk_abs_path(root_dir, template_dir, PQgetvalue(results, 0, 3), NULL);
	char *template_data = read_file(template_path);
	if (!template_data) {
		fprintf(stderr, "Unable to read template: %s\n", template_path);
		free(template_path);
		return -1;
	}
	free(template_path);
	// load up query_file
	char *query_path = mk_abs_path(root_dir, query_dir, PQgetvalue(results, 0, 4), NULL);
	char *query_file = read_file(query_path);
	if (!query_file) {
		fprintf(stderr, "Unable to load query: %s\n", query_path);
		free(query_path);
		return -1;
	}
	free(query_path);
	// render out pages
	int results_len = PQntuples(results);
	// TODO: Nothing 'is scalar' even pages that are, could/should be twisted
	// into using the 'vectorized' backend. (less code paths to manage).
	if (is_scalar(spec_id)) {
		handle_pages_scalar(
			conn, flipper, results, results_len, spec_id,
			template_data, query_file, global_context
		);
	} else {
		handle_pages_vector(
			conn, flipper, results, results_len, spec_id,
			template_data, &query_file, global_context
		);
	}
	free(template_data);
	free(query_file);
	PQclear(results);
	return 0;
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
	char *dir_name = dirname(dir);
	if (!file_exists(dir_name)) {
		if (mkdir_p(dir_name)) {
			fprintf(stderr, "error making directory: dir:%s \n", dir_name);
			ret =  -1;
		}
	}
	if(write_file(filename, data)) {
		fprintf(stderr, "write_page failed. filename: %s\n", filename);
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
	char *dir_name = dirname(dir);
	if (!file_exists(dir_name)) {
		if (mkdir_p(dir_name)) {
			fprintf(stderr, "error making directory: \"%s\"\n", dir_name);
			ret = -1;
		}
	}
	// parse out main from data
	char *start = strstr(data, "<main>");
	if (!start) {
		free(filename);
		free(dir);
		return -1;
	}
	char *end = strstr(start, "</main>");
	char *pdata  = calloc(strlen(data) + 1, sizeof(*pdata));
	strncpy(pdata, start, (end + 7) - start);
	if (write_file(filename, pdata)) {
		fprintf(stderr, "write_pjax failed. filename:%s\n", filename);
		ret = -1;
	}
	free(filename);
	free(dir);
	free(pdata);
	return ret;
}


#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <pthread.h>
#include <errno.h>

#include "deps/hash_table/hash_table.h"
#include "deps/hash_table/fnv_hash.h"

#include <json-c/json.h> 

#include "renderer.h"
#include "log.h"
#include "config.h"
#include "templates.h"
#include "files.h"
#include "flag_flipper.h"
#include "query.h"
#include "queries.h"

//---------------------------------------------------------
// TODO: move config values to db
// note: their duplicated throughout the codebase
// * they need to be coming out of the db.
// * program should accept a single parameter arg to a 
//   'config file' which contains a single db connection string value.
//    from there its all json/db based.
#define CONFIG_FILE "./renderer.conf"
#define CONN_INFO "port=5432 dbname=walden user=postgres"
#define LISTEN_CMD_1 "listen webpage_dirty"
#define LISTEN_CMD_2 "listen webpages_dirty"
#define CHUNK_SIZE 2000
/*
static char root_dir[] = "/var/html/c2v";
static char template_dir[] = "templates";
static char web_dir[] = "www";
static char query_dir[] = "queries";
*/
//---------------------------------------------------------

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

static int handle_pages_scalar(RendererState *self,
		PGresult *results, int results_len, int spec_id, char *template_data,
		char *query_file, json_object *global_context)
{
	fprintf(stderr, "START handle_pages_scalar \n");
	// query per page.
	Config *conf = configurator_get_config(self->configurator);
	PGresult *res;
	PageIdArray *page_ids = page_id_array_create(results_len);

	for (int i = 0; i < results_len; i++) {
		page_ids->data[i] = atoi(PQgetvalue(results, i, 0));
		page_ids->len++;
		char *cmd = strdup(query_file); // TODO: reuse a buffer instead of dup'n
		char *params = PQgetvalue(results, i, 5);
		rewrite_query(&cmd, params);
		res = PQexec(self->conn, cmd);
		if (PQresultStatus(res) != PGRES_TUPLES_OK) {
			fprintf(stderr, "get_query_result failed: cmd: %s  %s\n", cmd, PQerrorMessage(self->conn));
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
		json_object_object_add(obj, "__template_root__", json_object_new_string(conf->template_root));
		json_object_object_add(obj, "__web_root__", json_object_new_string(conf->web_root));
		json_object_object_merge(obj, global_context);
		// render
		char *html = render_template_str(template_data, obj);
		//write files
		const char *filename = PQgetvalue(results, i, 1);
		char *path = PQgetvalue(results, i, 2);
		if (strchr(path, '/')) {
			path += 5; // walk path forward past the first dir.
		} else {
			path += 4;
		}
		//fprintf(stderr, "filename: %s , path: %s\n", filename, path);
		// TODO:  pass in config->web_dir 
		if (write_page(conf->web_root, filename, path, html)) {
			fprintf(stderr, "unable to write html file. web_root:%s filename:%s path:%s\n", conf->web_root, filename, path);
		}
		if (write_pjax(conf->web_root, filename, path, html)) {
			fprintf(stderr, "unable to write pjax file. web_root:%s filename:%s path:%s\n", conf->web_root, filename, path);
		}
		free(html);
		json_object_put(obj);
		PQclear(res);
	}
	pthread_mutex_lock(&(self->flipper->ctl->mutex));
	// TODO: use a struct so we can store len along side the ids
	bqueue_push(self->flipper->wq, page_ids);
	pthread_mutex_unlock(&(self->flipper->ctl->mutex));
	fprintf(stderr, "END handle_pages_scalar \n");
	return 0;
}

//int augment_query(char **query_file, char **query_params, int params_len);
static int augment_query(char **query_file, char **query_params, int params_len)
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

static int handle_pages_vector(RendererState *self,
		PGresult *results, int results_len, int spec_id, char *template_data,
		char **query_file, json_object *global_context)
{
	fprintf(stderr, "START handle_pages_vector \n");
	if (!results_len) {
		fprintf(stderr, "no results to process\n");
		return 0;
	}
	fprintf(stderr, "results.len: %d\n", results_len);
	// 1 query for all pages.
	Config *conf = configurator_get_config(self->configurator);
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
	res = PQexec(self->conn, *query_file);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "error with query: %s error:%s\n",
				*query_file, PQerrorMessage(self->conn));
		fprintf(stderr, "END handle_pages_vector \n");
		return 0;
	}
	if (!PQntuples(res)) {
		fprintf(stderr, "vectorized query returned 0 rows. \n");
		fprintf(stderr, "error with query: %s error:%s\n",
				*query_file, PQerrorMessage(self->conn));
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
		json_object_object_add(obj, "__template_root__", json_object_new_string(conf->template_root));
		json_object_object_add(obj, "__web_root__", json_object_new_string(conf->web_root));
		json_object_object_merge(obj, global_context);
		// render
		char *html = render_template_str(template_data, obj);
		// write html files
		item = NULL;
		item = hash_table_search(ptable, PQgetvalue(res, i, 0));
		if (!item) {
			fprintf(stderr, "unable to find item for: %s\n", PQgetvalue(res, i, 0));
			continue;
		}
		int row_idx = *(int *)item->data; 
		const char *filename = PQgetvalue(results, row_idx, 1);
		char *path = PQgetvalue(results, row_idx, 2);
		if (strchr(path, '/')) {
			path += 5; // walk path forward past the first dir.
		} else {
			path += 4;
		}
		if (write_page(conf->web_root, filename, path, html)) {
			fprintf(stderr, "unable to write html file. web_root:%s filename:%s path:%s\n", conf->web_root, filename, path);
		}
		if (write_pjax(conf->web_root, filename, path, html)) {
			fprintf(stderr, "unable to write pjax file. web_root:%s filename:%s path:%s\n", conf->web_root, filename, path);
		}
		free(html);
		json_object_put(obj);
	}
	hash_table_destroy(ptable, NULL);
	PQclear(res);
	pthread_mutex_lock(&(self->flipper->ctl->mutex));
	bqueue_push(self->flipper->wq, page_ids);
	pthread_cond_broadcast(&(self->flipper->ctl->cond));
	pthread_mutex_unlock(&(self->flipper->ctl->mutex));
	fprintf(stderr, "END handle_pages_vector \n");
	return 0;
}

int handle_pages(RendererState *self, PGresult *results,
		int spec_id, json_object *global_context)
{
	// load up template_file
	Config *conf = configurator_get_config(self->configurator);
	char *template_path = mk_abs_path(conf->template_root, PQgetvalue(results, 0, 3), NULL);
	char *template_data = read_file(template_path);
	if (!template_data) {
		fprintf(stderr, "Unable to read template: %s\n", template_path);
		free(template_path);
		return -1;
	}
	free(template_path);
	// load up query_file
	char *query_path = mk_abs_path(conf->query_root, PQgetvalue(results, 0, 4), NULL);
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
			self, results, results_len, spec_id,
			template_data, query_file, global_context
		);
	} else {
		handle_pages_vector(
			self, results, results_len, spec_id,
			template_data, &query_file, global_context
		);
	}
	free(template_data);
	free(query_file);
	PQclear(results);
	return 0;
}

int write_page(const char *web_dir, const char *name,
		const char *path, const char *data)
{
	//fprintf(stderr, "write_page: name:\"%s\" path:\"%s\"\n", name, path);
	int ret = 0;
	char *filename = mk_abs_path((char *)web_dir, (char *)path, (char *)name, NULL);
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

int write_pjax(const char *web_dir, const char *name,
		const char *path, const char *data)
{
	//fprintf(stderr, "write_pjax: name:\"%s\" path:\"%s\"\n", name, path);
	int ret = 0;
	char pjax_dir[] = "_";
	char *filename = mk_abs_path((char *)web_dir, pjax_dir, (char *)path, (char *)name, NULL);
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
		fprintf(stderr, "write_pjax: error parsing <main>\n");
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

static json_object *get_global_context(RendererState *state)
{
	PGresult *res;
	res = PQexecPrepared(state->conn, "get-global-context", 0, NULL, NULL, NULL, 0);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "get-global-context Error: %s \n", PQerrorMessage(state->conn));
	}
	json_object *global_context = json_tokener_parse(PQgetvalue(res, 0, 0));
	PQclear(res);
	return global_context;
}

static int init_postgres(RendererState *state, const char *conninfo)
{
	// connect to db	
	PGresult *res;
	state->conn = PQconnectdb(conninfo);
	if (PQstatus(state->conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s",
				PQerrorMessage(state->conn));
		return -1;
	}
	fprintf(stderr, "Connection to db succeeded: %s\n", conninfo);
	// set search_path	
	res = PQexec(state->conn, "set search_path = walden"); 
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "set search_path failed: %s\n", PQerrorMessage(state->conn));
		PQclear(res);
		return -1;
	}
	PQclear(res);
	// setup listen command's
	res = PQexec(state->conn, LISTEN_CMD_1);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "listen command failed: %s\n", PQerrorMessage(state->conn));
		PQclear(res);
		return -1;
	}
	PQclear(res);
	res = PQexec(state->conn, LISTEN_CMD_2);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "listen command failed: %s\n", PQerrorMessage(state->conn));
		PQclear(res);
		return -1;
	}
	PQclear(res);
	// setup prepared statements
	res = PQprepare(state->conn, "get-dirty-spec-ids", (const char *)get_spec_ids_sql, 1, NULL);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "failed to prepare statement: %s\n",
				PQerrorMessage(state->conn));
		PQclear(res);
		return -1;
	}
	PQclear(res);
	res = PQprepare(state->conn, "get-dirty-pages", (const char *)get_dirty_sql, 1, NULL);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "failed to prepare statement: %s\n",
				PQerrorMessage(state->conn));
		PQclear(res);
		return -1;
	}
	PQclear(res);
	res = PQprepare(state->conn, "get-global-context", (const char*)get_global_context_sql, 1, NULL);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "failed to prepare statement: %s\n",
				PQerrorMessage(state->conn));
		PQclear(res);
		return -1;
	}
	PQclear(res);
	return 0;
}

static void multi_page(RendererState *state, PGnotify *notify)
{
	fprintf(stderr, "multi_page START: %s\n", get_formatted_time()); 
	json_object *global_context = get_global_context(state);
	const char *params[2];
	char *pk = NULL;
	PGresult *res;
	res = PQexecPrepared(state->conn, "get-dirty-spec-ids", 0, NULL, NULL, NULL, 0);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "get-dirty-spec-ids error: %s \n", PQerrorMessage(state->conn));
	}
	int spec_ids_len = PQntuples(res);
	if (!spec_ids_len) { return ;} // bail if nothing to do
	char *spec_ids[spec_ids_len];
	for (int i=0; i < spec_ids_len; i++) {
		spec_ids[i] = PQgetvalue(res, i, 0);
	}
	for (int i = 0; i < spec_ids_len; i++) {
		fprintf(stderr, "multi_page [start spec] spec_id: %s time: %s\n", spec_ids[i], get_formatted_time()); 
		int res_cnt = 0;
		params[0] = spec_ids[i];
		params[1] = "0";
		bool has_more = true;
		while (has_more) {
			PGresult *res2 = PQexecPrepared(state->conn, "get-dirty-pages", 2, params, NULL, NULL, 0);
			if (PQresultStatus(res2) != PGRES_TUPLES_OK) {
				fprintf(stderr, "get-dirty-pages params: %s, error: %s \n", *params, PQerrorMessage(state->conn));
				break;
			}
			//fprintf(stdout, "res_cnt: %d\n", res_cnt);
			if (pk) {free(pk); pk = NULL;}
			int res_len = PQntuples(res2);
			if (!res_len) {
				has_more = false;
				PQclear(res2);
				break;
			}
			res_cnt += res_len;
			has_more = res_len == CHUNK_SIZE;
			if (has_more) {
				// TODO: free params[1]
				pk = strdup(PQgetvalue(res2, res_len-1, 0));
				params[1] = pk; 
			}
			//fprintf(stderr, "has_more: %d, params[1]: %s\n", has_more, params[1]);
			// TODO: this becomes a queue write when we go multi-threaded.
			fprintf(stderr, "multi_page [start handle_pages] spec_id: %s pk:%s res_cnt: %d time: %s\n", spec_ids[i], params[1], res_cnt, get_formatted_time()); 
			handle_pages(state, res2, atoi(spec_ids[i]), global_context);
			fprintf(stderr, "multi_page [end handle_pages] spec_id: %s pk:%s res_cnt: %d time: %s\n", spec_ids[i], params[1], res_cnt, get_formatted_time()); 
			// TODO: figure out column/row
			//fprintf(stderr, "step of %d items %s\n", CHUNK_SIZE, get_formatted_time()); 
		}
		fprintf(stderr, "multi_page [end spec] spec_id: %s time: %s\n", spec_ids[i], get_formatted_time()); 
	}
	json_object_put(global_context);
	PQclear(res);
	//fprintf(stderr, "updated page: %s \n", notify->extra);
	fprintf(stderr, "multi_page END: %s\n", get_formatted_time()); 
}

static RendererState *renderer_create(void)
{
	fprintf(stderr, "renderer_create\n");
	RendererState *state = malloc(sizeof(*state));
	state->configurator = configurator_aloc();
	configurator_conf(state->configurator, CONFIG_FILE);
	//
	// make background worker for updating dirty flags.
	state->flipper = flag_flipper_new();
	controller_activate(state->flipper->ctl);	
	if (pthread_create(&state->tid, NULL, webpage_clear_dirty_thread,
				(void *)state->flipper)) {
		fprintf(stderr, "unable to start worker thread. \n");
		return NULL;
	}
	if (init_postgres(state, CONN_INFO)) {
		return NULL;
	}
	state->run = true;
	return state;
}

static void renderer_delete(RendererState *state)
{
	fprintf(stderr, "clean_up start.\n");
	pthread_cond_signal(&(state->flipper->ctl->cond));
	controller_deactivate(state->flipper->ctl);
	pthread_join(state->tid, NULL);
	fprintf(stderr, "pthread_join finished.\n");
	PQfinish(state->conn);
	fprintf(stderr, "clean_up finish.\n");
	fprintf(stderr, "renderer_delete\n");
	free(state);
	state = NULL;
}

static void renderer_unload(RendererState *state)
{
	fprintf(stderr, "renderer_unload\n");
}

static void renderer_reload(RendererState *state)
{
	fprintf(stderr, "renderer_reload\n");
}

static bool renderer_update(RendererState *state)
{
	fprintf(stderr, "renderer_update\n");
	PGnotify *notify;
	int sock = PQsocket(state->conn);
	fd_set input_mask;

	if (sock < 0) {
		fprintf(stderr, "sock < 0\n Quitting!!!\n");
		return state->run = false;
	}
	FD_ZERO(&input_mask);
	FD_SET(sock, &input_mask);
	// TODO: use a timeout
	// return of: 0 is timeout expired
	// return of: -1 is error
	// return of: > 0 is # of bits in readfds
	if (select(sock + 1, &input_mask, NULL, NULL, NULL) < 0) {
		fprintf(stderr, "select() failed: %s\n", strerror(errno));
		return state->run = false;
	}
	PQconsumeInput(state->conn);
	while ((notify = PQnotifies(state->conn))) {
		fprintf(stderr, "notification: relname: %s \n", notify->relname);
		fprintf(stderr, "notify START: %s\n", get_formatted_time()); 
		if(!strcmp(notify->relname, "webpages_dirty")) {
			multi_page(state, notify);
		} else {
			fprintf(stderr, "notify unknown relname: %s\n", notify->relname);	
		}
		PQfreemem(notify);
		fprintf(stderr, "notify END: %s\n", get_formatted_time()); 
	}
	// purge the leftovers out of the queue.
	pthread_cond_signal(&state->flipper->ctl->cond);
	return state->run;
}

const RendererApi renderer_api = {
	.create = renderer_create,
	.delete = renderer_delete,
	.unload = renderer_unload,
	.reload = renderer_reload,
	.update = renderer_update
};

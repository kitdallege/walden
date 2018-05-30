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
 * TODO: 
 *  - config file for settings rather than being hard coded!
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <linux/limits.h>

#include <libpq-fe.h>

#include "mustach-json-c.h"
#include <json-c/json.h> 

// #define CONN_INFO "port=5555 dbname=test-db user=kit"
#define CONN_INFO "port=5432 dbname=c2v user=c2v_admin"
#define LISTEN_CMD "listen dirty_webpage"

static char root_dir[] = "/var/html/c2v";
static char template_dir[] = "templates";
static char web_dir[] = "www";
static char query_dir[] = "queries";
static uid_t user = 33;
static gid_t group = 33;
// TODO: perms are 0666 . this should prob be tighen down @ some point to 0655
static mode_t perms = DEFFILEMODE; 
static mode_t dir_perms = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
static void exit_nicely(PGconn *conn)
{
	PQfinish(conn);
	exit(1);
}
/*
 *{ "id" : "1422",
 *  "filename" : "24.html", 
 *  "path" : "root/events/events/2012/07", 
 *  "template" : "events.mustache", 
 *  "query" : "events.sql", 
 *  "query_params" : ""
 *}
*/
typedef struct page_spec 
{
	unsigned int id;
	const char *filename;
	char *path;
	char *template;
	char *query;
	const char *query_params;
} page_spec;

static char *read_file(const char *filename)
{
	char *data;
	struct stat st;
	int fd;
	fd = open(filename, O_RDONLY);
	fstat(fd, &st);
	data = malloc(st.st_size + 1);
	memset(data, 0, st.st_size + 1);
	read(fd, data, st.st_size);
	close(fd);
	return data;
}
/* each positional argument is assumed to be a path component which will
 * be joined via '/' together. the last argument must be null, as its is 
 * used as a sentinal.
 * A char * is allocated and returned, it is up to the caller to free it!
 * ex: char *str = mk_abs_path(var1, var2, var3, null);
 */
static char *mk_abs_path(char *base, ...)
{
        unsigned int count = 0;
        unsigned int size = 0;
        size_t s_len;
        char *p, *str, *dest;
        va_list args;
        // compute required memory
        str = (char *)base;
        va_start(args, base);
        while (str) {
                size += strlen(str); 
                count++;
                str = va_arg(args, char *);
        }
        va_end(args);
        // allocate memory and copy args into dest separated by delimter.
        p = dest = malloc(sizeof(char *) * (size + count + 1));
        va_start(args, base);
        str = (char *)base;
        for (unsigned int i = 0; i < count; i++) {
                s_len = strlen(str);
		if (s_len) { 
			if (i) { *p++ = '/'; }
			memcpy(p, str, s_len);
			p += s_len;
		}
                str = va_arg(args, char *);
        }
        va_end(args);
        *p = '\0';
        return dest;
}

/* TODO: any call to strdup needs to first test for 0x00 */
static page_spec *parse_page_spec(const char *payload)
{
	const char *temp;
	json_object *obj, *attr;
	obj = json_tokener_parse(payload);
	if (!obj) {
		return NULL;
	}
	page_spec *spec = malloc(sizeof(*spec));
	attr = json_object_object_get(obj, "id");
	spec->id = json_object_get_int(attr);
	attr = json_object_object_get(obj, "filename");
	spec->filename = strdup(json_object_get_string(attr));
	attr = json_object_object_get(obj, "path");
	temp = json_object_get_string(attr);
	//fprintf(stderr, "page_spec: temp:\"%s\"\n", temp);
	int offset = temp[4] == '/' ? 5 : 4;
	spec->path = strdup(temp + offset);
	attr = json_object_object_get(obj, "template");
	spec->template = mk_abs_path(root_dir, template_dir,
					json_object_get_string(attr), NULL);
	attr = json_object_object_get(obj, "query");
	spec->query = mk_abs_path(root_dir, query_dir,
					json_object_get_string(attr), NULL);
	attr = json_object_object_get(obj, "query_params");
	spec->query_params = strdup(json_object_get_string(attr));

	json_object_put(obj); // release the ref
	return spec;
}

static void free_page_spec(page_spec *spec)
{
	free((char *)spec->path);
	free((char *)spec->template);
	free((char *)spec->query);
	free(spec);
	spec = NULL;
}

static int file_exists(const char *filepath)
{
	struct stat st;
	return !stat(filepath, &st);
}

/* munges query string to append params.
 * how:
 *   remove ';' @ the end.
 *   scans until it finds ')', 
 *   cuts it, 
 *   adds params, 
 *   and then adds the ');' back
 * TODO: add handling for normal where clauses etc...
 */
static void rewrite_query(char **query, const char *params)
{
	if (!params || strlen(params) < 1) { return ;}
	//fprintf(stderr, "rewrite_query: \n q:\"%s\" \np:\"%s\"\n\n", *query, params);
	//TODO: use less of strlen & strcat.
	*query = realloc(*query, strlen(*query) + strlen(params) + 3);
	char *p = *query + strlen(*query) - 1;
	for (;*p != ')'; p--){};
	*p = '\0';
	strcat(*query, params);
	strcat(*query, ");");
	//fprintf(stderr, "rewrite_query: query:\n\"%s\"\n\n", *query);
}

static char *get_query_result(PGconn *conn, const char *file, const char *params)
{
	char *cmd = read_file(file);
	if (!cmd) {
		fprintf(stderr, "Unable to load query: %s\n", file);
		return NULL;
	}
	rewrite_query(&cmd, params);
	PGresult *res;
	res = PQexec(conn, cmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "get_query_result failed: cmd: %s  %s\n",
				cmd, PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	char *result = strdup(PQgetvalue(res, 0, 0));
	/* // handy for debugging
	 * PQprintOpt options = {0};
	 * options.header = 1;
	 * options.align = 1;
	 * options.fieldSep = "|";
	 * PQprint(stdout, result, &options)
	*/
	PQclear(res);
	return result;
}

static char *render_template(const char *template, const char *json_data)
{
	char *template_data = read_file(template);
	if (!template_data) {
		fprintf(stderr, "Unable to read template: %s\n", template);
		return NULL;
	}
	json_object *obj = json_tokener_parse(json_data);
	if (!obj) {
		fprintf(stderr, "Unabled to parse json_data. %s\n", json_data);
		free(template_data);
		return NULL;
	}
	char *result;
	size_t result_size;
	FILE *stream = open_memstream(&result, &result_size);
	if (fmustach_json_c(template_data, obj, stream)) {
		fprintf(stderr, "render failed. \n");
		free(template_data);
		json_object_put(obj);
		return NULL;
	}
	fflush(stream);
	fclose(stream);
	free(template_data);
	json_object_put(obj);
	return result;
}

/* Adapted from 
 * https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950
 */
static int mkdir_p(const char *path)
{
	const size_t len = strlen(path);
	char lpath[PATH_MAX];
	char *p;

	errno = 0;

	if (len > sizeof(lpath) - 1) {
		errno = ENAMETOOLONG;
		return -1;
	}

	strcpy(lpath, path);

	for (p = lpath + 1; *p; p++) {
		if (*p == '/') {
			*p = '\0';
			if (mkdir(lpath, dir_perms)) {
				if (errno != EEXIST) {
					return -1;
				}
			}
			*p = '/';
		}
	}
	if (mkdir(lpath, dir_perms)) {
		if (errno != EEXIST) {
			return -1;
		}
	}
	return 0;
}

/* use a temp file w/rename to make file writes atomic. */
static int write_file(const char *name, const char *data)
{
	// name + ~
	char temp_name[strlen(name) + 2];
	snprintf(temp_name, sizeof(temp_name), "%s~", name);
	// if it exists remove it
	if (unlink(temp_name)) {
		if (errno != ENOENT) {
			fprintf(stderr, "unable to remove existing temp file: %s\n", strerror(errno));
			return -1;	
		}
	}
	// open temp file
	int fd = open(temp_name, O_RDWR|O_CREAT|O_TRUNC, perms);
	if (fd == -1) {
		fprintf(stderr, "failed to open file for writing: %s\n", strerror(errno));
		return -1;
	}
	// write to it
	dprintf(fd, data);
	//  flush the buffer	
	if (fsync(fd)) {
		fprintf(stderr, "failed to fsync file: %s\n", strerror(errno));
		return -1;
	}
	// change owner & group
	if (fchown(fd, user, group)) {
		fprintf(stderr, "failed to fsync file: %s\n", strerror(errno));
		return -1;
	}
	// set file perms
	if (fchmod(fd, perms)) {
		fprintf(stderr, "failed to chmod file: %s\n", strerror(errno));
		return -1;
	}
	if (close(fd)) {
		return -1;
	}
	// *atomic write via rename* [keep from causing an nginx error]
	if (rename(temp_name, name)) {
		fprintf(stderr, "failed to rename file: %s\n", strerror(errno));
	   return -1;
	}
	return 0;
}

static int write_page(const char *name, const char *path, const char *data)
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

static int write_pjax(const char *name, const char *path, const char *data)
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

static int webpage_clear_dirty_flag(PGconn *conn, int id)
{
	char cmd[128]; // stmt w/sys.maxsize: 87
       	sprintf(cmd, "update webpage set date_updated = default, dirty = false "
			"where id = %d", id);
	PGresult *res;
	res = PQexec(conn, cmd);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "webpage_clear_dirty_flag failed: %s\n",
				PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);
	return 0;
}

static int handle_page(PGconn *conn, const char *payload)
{
	// parse payload into a struct
	page_spec *spec = parse_page_spec(payload);
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
		free(spec);
		return 1;
	};
	// run query and get text response 
	char *json_data = get_query_result(conn, spec->query, spec->query_params);
	if (!json_data) {
		fprintf(stderr, "query returned no data:%s\n", spec->query);
		free(spec);
		return 1;
	}
	// render template and get html text
	char *html = render_template(spec->template, json_data);
	if (!html) {
		fprintf(stderr, "render_template had zero length result.\n");
		free(spec);
		//free(json_data);
		return 1;
	}
	// write html to disk
	if (write_page(spec->filename, spec->path, html)) {
		fprintf(stderr, "unable to write html file.\n");
		free(spec);
		free(json_data);
		free(html);
		return 1;
	}
	// write pjax (<main> block of html) to '_' directory
	if (write_pjax(spec->filename, spec->path, html)) {
		fprintf(stderr, "unable to write pjax file.\n");
		free(spec);
		free(json_data);
		free(html);
		return 1;
	}
	// update webpage setting dirty flag to false.
	if (webpage_clear_dirty_flag(conn, spec->id)) {
		fprintf(stderr, "unable to clear dirty flag: page{id=%d}\n",
				spec->id);
		free(spec);
		free(json_data);
		free(html);
		return 1;
	}
	free(spec);
	free(json_data);
	free(html);
	return 0;
}
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

int main(int argc, char **argv)
{
	const char *conninfo = CONN_INFO; 
	PGconn *conn;
	PGresult *res;
	PGnotify *notify;
	int quit = 0;

	conn = PQconnectdb(conninfo);

	if (PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s",
				PQerrorMessage(conn));
		exit_nicely(conn);
	}

	//res = PQexec(conn, "select pg_catalog.set_config('search_path', 'public', false)");
	res = PQexec(conn, "set search_path = public"); 
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "SET failed: %s\n", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

	res = PQexec(conn, LISTEN_CMD);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "Listen command failed: %s\n",
				PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);
	
	while (!quit) {
		int sock = PQsocket(conn);
		fd_set input_mask;

		if (sock < 0) { break; }
		FD_ZERO(&input_mask);
		FD_SET(sock, &input_mask);

		if (select(sock + 1, &input_mask, NULL, NULL, NULL) < 0) {
			fprintf(stderr, "select() failed: %s\n", strerror(errno));
			exit_nicely(conn);
		}
		PQconsumeInput(conn);
		while ((notify = PQnotifies(conn))) {
			//fprintf(stderr, "ASYNC NOTIFY of '%s' received from backend PID %d with a payload of: %s\n", notify->relname, notify->be_pid, notify->extra);
			timespec ct1, ct2, pt1, pt2, td;
			clock_gettime(CLOCK_MONOTONIC, &ct1);
			clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt1);
			//clock_t ticks, new_ticks;
			//ticks = clock();
			if (handle_page(conn, notify->extra)) {
				fprintf(stderr, "handle_page error on: %s \n",
						notify->extra);
			} else {
				//new_ticks = clock();
				//double elapsed = (double)(new_ticks - ticks) * 1000.0 / CLOCKS_PER_SEC;
				clock_gettime(CLOCK_MONOTONIC, &ct2);
				clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pt2);
				fprintf(stderr, "updated page: %s \n", notify->extra);
				td = diff(ct1, ct2);
				fprintf(stderr, "system: time elapsed: %.3f msec / %ld ns\n", td.tv_nsec / 1000000.0, td.tv_nsec); 
				td = diff(pt1, pt2);
				fprintf(stderr, "cpu: time elapsed: %.3f msec / %ld ns\n", td.tv_nsec / 1000000.0, td.tv_nsec); 

			}
			PQfreemem(notify);
		}
	}
	fprintf(stderr, "Done.\n");
	PQfinish(conn);
	return 0;
}


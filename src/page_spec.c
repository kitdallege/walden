#include <string.h>
#include <json-c/json.h> 

#include "page_spec.h"
#include "files.h"

static char root_dir[] = "/var/html/c2v";
static char template_dir[] = "templates";
//static char web_dir[] = "www";
static char query_dir[] = "queries";

/*
 *{ "id" : "1422",
 *  "filename" : "24.html", 
 *  "path" : "root/events/events/2012/07", 
 *  "template" : "events.mustache", 
 *  "query" : "events.sql", 
 *  "query_params" : ""
 *}
*/
PageSpec *parse_page_spec(const char *payload)
{
	const char *temp;
	json_object *obj, *attr;
	obj = json_tokener_parse(payload);
	if (!obj) {
		return NULL;
	}
	PageSpec *spec = malloc(sizeof(*spec));
	attr = json_object_object_get(obj, "id");
	spec->id = json_object_get_int(attr);
	attr = json_object_object_get(obj, "filename");
	spec->filename = strdup(json_object_get_string(attr));

	attr = json_object_object_get(obj, "path");
	temp = json_object_get_string(attr);
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

void free_page_spec(PageSpec *spec)
{
	free((char *)spec->path);
	free((char *)spec->template);
	free((char *)spec->filename);
	free((char *)spec->query);
	free((char *)spec->query_params);
	free(spec);
	spec = NULL;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mustach-json-c.h"
#include <json-c/json.h> 

#include "files.h"
#include "templates.h"

char *render_template(const char *template, const char *json_data)
{
	// TODO: pass in template_data rather than the path.
	char *template_data = read_file(template);
	if (!template_data) {
		fprintf(stderr, "Unable to read template: %s\n", template);
		return NULL;
	}
	char *results = render_template_str(template_data, json_data);
	free(template_data);
	return results;
}

char *render_template_str(const char *template_data, const char *json_data)
{
	// TODO: pass in template_data rather than the path.
	if (!template_data || !strlen(template_data)) {
		fprintf(stderr, "Template was empty: n");
		return NULL;
	}
	// TODO: look into custom malloc with backing pool for json parser.
	json_object *obj = json_tokener_parse(json_data);
	if (!obj) {
		fprintf(stderr, "Unabled to parse json_data. %s\n", json_data);
		return NULL;
	}
	json_object *cwd = json_object_new_string("/var/html/c2v/templates");
	json_object_object_add(obj, "CWD", cwd);
	char *result;
	size_t result_size;
	// TODO: pass in memstream (open/close) in caller.
	FILE *stream = open_memstream(&result, &result_size);
	if (fmustach_json_c(template_data, obj, stream)) {
		fprintf(stderr, "render failed. \n");
		json_object_put(obj);
		return NULL;
	}
	fflush(stream);
	fclose(stream);
	json_object_put(obj);
	return result;
}


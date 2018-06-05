#include <stdio.h>


#include "mustach-json-c.h"
#include <json-c/json.h> 

#include "files.h"
#include "templates.h"

char *render_template(const char *template, const char *json_data)
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


#ifndef TEMPLATES_H 
#define TEMPLATES_H

#include <json-c/json.h> 

char *render_template(const char *template, const char *json_data);
char *render_template_str(const char *template_data, json_object *obj);
void json_object_object_merge(json_object *obj1, json_object *obj2);

#endif


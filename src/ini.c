#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "ini.h"

static char *readline(FILE *file)
{
	size_t len   = 0;
	size_t size  = 0;
	size_t last  = 0;
	char *buffer = NULL;

	while (!feof(file)) {
		size += BUFSIZ;
		buffer = realloc(buffer, size);
		memset(buffer, 0, size);
		fgets(buffer+last, size, file);
		len = strlen(buffer);
		last = len - 1;
		//TODO: combine ifs  & return the '\n' instead of remove it.
		if (!len) {
				break;
		}
		if (buffer[last] == '\n') {
				buffer[last] = '\0';
				break;
		}
	}
	return buffer;
}

char *ini_get_db_conf_from_file(const char *filename)
{
	FILE *file;
	char *line;
	int line_len;
	char *value = NULL, *p;
	file = fopen(filename, "r");
	if (!file) {
		fprintf(stderr, "unable to open ini file: %s\n", filename);
		fprintf(stderr, "%s\n", strerror(errno));
	}
	while((line=readline(file))) {
		line_len = strlen(line);
		if (!line_len || line[0] == ';' || !strstr(line, "db-address")) {
			free(line);
			continue;
		}
		line[line_len] = '\0';
		p = strchr(line, '=');
		p++;
		while (*p == ' ' || *p == '"') {p++;}
		value = malloc(line_len);
	   	strcpy(value, p);
		p = &value[strlen(p)-1];
		while (*p == ' ' || *p == '"') {*p-- = '\0';}
		fprintf(stderr, "ini_get_db_conf_from_file: \"%s\"\n", value);
		free(line);
		break;
	}
	fclose(file);
	return value;
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ./deps
#include "inih/ini.h"

#include "config.h"

struct Configurator
{
	char config_file[CONF_VALUE_LEN];
	Config *conf;
};

static int ini_parse_handler( void *user, const char *section,
		const char *name, const char *value)
{
	Config *config = (Config *)user;
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (MATCH("", "db-address")) {
		strcpy(config->db_conn_info, value);
	} else if (MATCH("template", "root")) {
		strcpy(config->template_root, value);
	} else if (MATCH("query", "root")) {
		strcpy(config->query_root, value);
	} else {
		return 0;
	}
	return 1;
}
/*
int load_config(const char *conf_file, Config *config)
{
	if (ini_parse(CONF_FILE, ini_parse_handler, config) < 0) {
		fprintf(stderr, "error loading config file.\n");
		return -1;
	}
	fprintf(stderr,
			"AppConfig {db_conn_info=%s, template_root=%s, query_root=%s} @ %p \n",
			config->db_conn_info, config->template_root,
			config->query_root, (void *)config);
	return 0;
}
*/

Configurator *configurator_aloc()
{
	Configurator *self = calloc(1, sizeof *self);
	self->conf = calloc(1, sizeof *self->conf);
	return self;
}

void configurator_conf(Configurator *self, void *user)
{
	strcpy(self->config_file, (const char *)user);
	if (configurator_load_config(self)) {
		fprintf(stderr, "error loading config file: %s\n", self->config_file);
	}
}

void configurator_zero(Configurator *self)
{
	// TODO: memset(self->config, 0, sizeof *self->config) ???
	memset(self->conf->db_conn_info, 0, CONF_VALUE_LEN);
	memset(self->conf->template_root, 0, CONF_VALUE_LEN);
	memset(self->conf->query_root, 0, CONF_VALUE_LEN);
}

void configurator_free(Configurator *self)
{
	free(self->conf);
	free(self);
}

// API
int configurator_load_config(Configurator *self)
{
	if (ini_parse(self->config_file, ini_parse_handler, self->conf) < 0) {
		fprintf(stderr, "error loading config file.\n");
		return -1;
	}
	fprintf(stderr,
			"AppConfig {db_conn_info=%s, template_root=%s, query_root=%s} @ %p \n",
			self->conf->db_conn_info, self->conf->template_root,
			self->conf->query_root, (void *)self->conf);
	return 0;

}

Config *configurator_get_config(Configurator *self)
{
	return self->conf;
}


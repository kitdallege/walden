#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libpq-fe.h>
#include <json-c/json.h>

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
	}
	return 1;
}

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
	// hit the db and 
}

void configurator_zero(Configurator *self)
{
	// TODO: memset(self->config, 0, sizeof *self->config) ???
	memset(self->conf->db_conn_info, 0, CONF_VALUE_LEN);
	memset(self->conf->template_root, 0, CONF_VALUE_LEN);
	memset(self->conf->query_root, 0, CONF_VALUE_LEN);
	self->conf->site_id = 0;
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
	// hit the db and load the 'config' now that we have the address.
	fprintf(stderr, "Connecting to: %s\n", self->conf->db_conn_info);
	PGconn *conn = PQconnectdb(self->conf->db_conn_info);
	if (PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s",
				PQerrorMessage(conn));
		return 1;
	}
	fprintf(stderr, "db conn successful. \n");
	PGresult *res = PQexec(conn, "set search_path = walden"); 
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "SET failed: %s\n", PQerrorMessage(conn));
		PQclear(res);
		return 1;
	}
	PQclear(res);
	fprintf(stderr, "Set search_path = walden successful.\n");
	res = PQexec(conn, "select value from site_setting where name = 'resource-manager';");
	char *json_data = PQgetvalue(res, 0, 0);
	json_object *obj = json_tokener_parse(json_data);
	json_object *attr;
	// template_dir, query_dir, site_id
	if (!json_object_object_get_ex(obj, "template_dir", &attr)) {
		fprintf(stderr, "unable to find template_dir in config json.\n");
		return 1;
	}
	strcpy(self->conf->template_root,  json_object_get_string(attr));
	if (!json_object_object_get_ex(obj, "query_dir", &attr)) {
		fprintf(stderr, "unable to find query_root in config json.\n");
		return 1;
	}
	strcpy(self->conf->query_root, json_object_get_string(attr));
	if (!json_object_object_get_ex(obj, "site_id", &attr)) {
		fprintf(stderr, "unable to find site_id in config json.\n");
		return 1;
	}
	self->conf->site_id = json_object_get_int(attr);
	fprintf(stderr,
			"AppConfig {db_conn_info=%s, template_root=%s, query_root=%s, site_id=%d} @ %p \n",
			self->conf->db_conn_info, self->conf->template_root,
			self->conf->query_root, self->conf->site_id, (void *)self->conf);
	PQfinish(conn);
	json_object_put(obj);
	return 0;

}

Config *configurator_get_config(Configurator *self)
{
	return self->conf;
}


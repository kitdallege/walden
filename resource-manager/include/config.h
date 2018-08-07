#ifndef CONFIG_H
#define CONFIG_H

#define CONF_VALUE_LEN 256

typedef struct Config
{
	char site_id[64];
	char db_conn_info[CONF_VALUE_LEN];
	char template_root[CONF_VALUE_LEN];
	char query_root[CONF_VALUE_LEN];
} Config;

typedef struct Configurator Configurator;

// live cycle
Configurator   *configurator_aloc(void);
void			configurator_conf(Configurator *self, void *user);
void			configurator_zero(Configurator *self);
void			configurator_free(Configurator *self);

// API
int		configurator_load_config(Configurator *self);
Config *configurator_get_config(Configurator *self);

#endif

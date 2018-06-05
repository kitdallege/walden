#ifndef RENDERER_H
#define RENDERER_H

#include <libpq-fe.h>
#include "flag_flipper.h"


int handle_page(PGconn *conn, flag_flipper_state *flipper, const char *payload);
int write_page(const char *name, const char *path, const char *data);
int write_pjax(const char *name, const char *path, const char *data);

#endif

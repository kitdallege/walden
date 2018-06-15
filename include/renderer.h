#ifndef RENDERER_H
#define RENDERER_H

#include <libpq-fe.h>
#include "flag_flipper.h"

int handle_page(PGconn *conn, FlagFlipperState *flipper, const char *payload);
int write_page(const char *name, const char *path, const char *data);
int write_pjax(const char *name, const char *path, const char *data);

// vectorized version of above.
//int page_handler_thread(PGconn *conn, FlagFlipperState *flipper, PGresult **results);
/*
 * main gets notification of 'dirty_pages'
 *  * query to get dirty pages / grouped | sorted
 *  * chunk out to a work queue
 *  ** so queue = [[results], [results], [results]]
 */

#endif


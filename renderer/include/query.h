#ifndef QUERY_H 
#define QUERY_H

#include <libpq-fe.h>

void rewrite_query(char **query, const char *params);
char *get_query_result(PGconn *conn, const char *file, const char *params);

#endif


#include <string.h>
#include <stdlib.h>

#include "files.h"
#include "query.h"

static void rewrite_query(char **query, const char *params);

/* munges query string to append params.
 * how:
 *   remove ';' @ the end.
 *   scans until it finds ')', 
 *   cuts it, 
 *   adds params, 
 *   and then adds the ');' back
 * TODO: add handling for normal where clauses etc...
 */
static void rewrite_query(char **query, const char *params)
{
	if (!params || strlen(params) < 1) { return ;}
	//fprintf(stderr, "rewrite_query: \n q:\"%s\" \np:\"%s\"\n\n", *query, params);
	//TODO: use less of strlen & strcat.
	*query = realloc(*query, strlen(*query) + strlen(params) + 3);
	char *p = *query + strlen(*query) - 1;
	for (;*p != ')'; p--){};
	*p = '\0';
	strcat(*query, params);
	strcat(*query, ");");
	//fprintf(stderr, "rewrite_query: query:\n\"%s\"\n\n", *query);
}

char *get_query_result(PGconn *conn, const char *file, const char *params)
{
	char *cmd = read_file(file);
	if (!cmd) {
		fprintf(stderr, "Unable to load query: %s\n", file);
		return NULL;
	}
	rewrite_query(&cmd, params);
	PGresult *res;
	res = PQexec(conn, cmd);
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		fprintf(stderr, "get_query_result failed: cmd: %s  %s\n",
				cmd, PQerrorMessage(conn));
		PQclear(res);
		free(cmd);
		return NULL;
	}
	char *result = strdup(PQgetvalue(res, 0, 0));
	/* // handy for debugging
	 * PQprintOpt options = {0};
	 * options.header = 1;
	 * options.align = 1;
	 * options.fieldSep = "|";
	 * PQprint(stdout, result, &options)
	*/
	PQclear(res);
	free(cmd);
	return result;
}



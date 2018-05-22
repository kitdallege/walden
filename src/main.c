/*
 * Program which renders a webpage when told via postgres (listen/notify).
 * 
 * TODO: 
 *  - Simple config file for resource locations as well as output.
 *  - Render function 
 *    - query webpage table
 *    - query context data (convert to json-c)
 *    - write to disk (w/atomic move) 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

#include <libpq-fe.h>


static void
exit_nicely(PGconn *conn)
{
	PQfinish(conn);
	exit(1);
}


int main(int argc, char **argv)
{
	const char *conninfo = "port=5555 dbname=test-db";
	PGconn *conn;
	PGresult *res;
	PGnotify *notify;
	int quit = 0;

	conn = PQconnectdb(conninfo);

	if (PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s",
					PQerrorMessage(conn));
		exit_nicely(conn);
	}

	//res = PQexec(conn, "select pg_catalog.set_config('search_path', 'public', false)");
	res = PQexec(conn, "set search_path = public"); 
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "SET failed: %s\n", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);

	res = PQexec(conn, "listen dirty_webpage");
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "Listen command failed: %s\n", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}
	PQclear(res);
	
	while (!quit) {
		int	sock = PQsocket(conn);
		fd_set	input_mask;

		if (sock < 0) {
			break;
		}
		FD_ZERO(&input_mask);
		FD_SET(sock, &input_mask);

		if (select(sock + 1, &input_mask, NULL, NULL, NULL) < 0) {
			fprintf(stderr, "select() failed: %s\n", strerror(errno));
			exit_nicely(conn);
		}
		/*
		 * Should extra just hold the json we care about. thus removing
		 * a query. (still have to query to update dirty flag)
		 */
		PQconsumeInput(conn);
		while ((notify = PQnotifies(conn))) {
			fprintf(stderr, "ASYNC NOTIFY of '%s' received from backend PID %d with a payload of: %s\n",
					notify->relname, notify->be_pid, notify->extra);
			PQfreemem(notify);
		}
	}
	fprintf(stderr, "Done.\n");
	PQfinish(conn);

	return 0;
}


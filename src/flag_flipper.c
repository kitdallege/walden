#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "flag_flipper.h"

flag_flipper_state *flag_flipper_new(void)
{
	flag_flipper_state *st = malloc(sizeof(*st));
	controller *ctl = malloc(sizeof(*ctl));
	controller_init(ctl);
	st->ctl = *ctl;
	st->wq = *queue_new();
	return st;
}

/* return'd char * is a comma separated list of the int arr[] arg..
 * ex: "#,#,#,#"
 * note: no trailing comma. also no comma on single element lists.
 */
static char *array_to_str(unsigned int arr[], size_t len)
{
	int d, num, d_idx = 0;
	char temp[21] = { 0 };
	char *p, *bp, *buf = calloc(20 * len + 1, sizeof(*buf));
	bp = buf;
	for (unsigned int i = 0; i < len; i++) {
		if (i) { *bp++ = ','; }
		num = arr[i];
		p = temp;
		// divide by 10 each iteration storing digit. temp ends up reversed.
		do { 
			d = num % 10;
			*p++ = (char)(d+48);
			num = num / 10;
			d_idx++;
		} while (num > 0);
		*p++ = '\0';
		while (d_idx) { *bp++ = temp[--d_idx]; } // (un)reverse temp and store
	}
	return buf;
}

void *webpage_clear_dirty_thread(void *arg)
{
	flag_flipper_state *st = (flag_flipper_state *)arg;
	// create a PGconn locally.
	PGresult *res;
	PGconn *conn = PQconnectdb("port=5432 dbname=c2v user=c2v_admin");
	if (PQstatus(conn) != CONNECTION_OK) {
		fprintf(stderr, "Connection to database failed: %s",
				PQerrorMessage(conn));
	}
	fprintf(stderr, "Connection to database succeeded\n");
	res = PQexec(conn, "set search_path = c2v"); 
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "SET failed: %s\n", PQerrorMessage(conn));
		PQclear(res);
	}
	fprintf(stderr, "Set search_path = public \n");
	PQclear(res);
	// build query out of passed in ids.
	//char id_str_temp[8];
	char *cmd = malloc(sizeof(*cmd) * 21100);
	char *id_str; // = malloc(sizeof(*id_str) * 21000);
	unsigned int count;
	unsigned int work;
	unsigned int ids[1000];	
	while (st->ctl.active) {
		//while (!st->wq.head && st->wq.size < 100 && st->ctl.active) {
		while ((st->wq.size < 900  && !st->ctl.force) && st->ctl.active) {
			fprintf(stderr, "size: %lu, force: %d, active: %d : ", st->wq.size, st->ctl.force, st->ctl.active);
			pthread_cond_wait(&st->ctl.cond, &st->ctl.mutex);
		}
		if (!st->ctl.active) { break; }
		if (st->ctl.force) {
			st->ctl.force = 0;
		}
		fprintf(stderr, "size: %lu, force: %d, active: %d : ", st->wq.size, st->ctl.force, st->ctl.active);
		// rip down an array < 1000 of ids.
		work = (unsigned int)queue_get(&st->wq); 
		for (count = 0; count < 1000 && work; count++) {
			ids[count] = work;
			work = (unsigned int)queue_get(&st->wq);
		}
		if (!count) {
			//fprintf(stderr, "skipping count of 0.\n");
			continue;
		}
		pthread_mutex_unlock(&st->ctl.mutex);
		// do stuff
		id_str = array_to_str(ids, count);
		memset(cmd, 0, sizeof(*cmd) * 21100);
       	sprintf(cmd, "update webpage set date_updated = default, dirty = false "
			"where id in (%s);", id_str);
		free(id_str);
		fprintf(stderr, "cmd: %s\n", cmd);
		fprintf(stderr, "updating %u page(s).\n", count);
		res = PQexec(conn, cmd);
		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			fprintf(stderr, "webpage_clear_dirty_flag failed: %s\n", PQerrorMessage(conn));
			PQclear(res);
		}
		PQclear(res);
		pthread_mutex_lock(&st->ctl.mutex);
	}
	fprintf(stderr, "flag flipper shutting down..\n");

	free(cmd);
	//free(id_str);
	//free(&st->wq); // causes error ?
	controller_destory(&st->ctl);
	free(&st->ctl);
	return NULL;
}

int webpage_clear_dirty_flag(PGconn *conn, int id)
{
	char cmd[128]; // stmt w/sys.maxsize: 87
       	sprintf(cmd, "update webpage set date_updated = default, dirty = false "
			"where id = %d", id);
	PGresult *res;
	res = PQexec(conn, cmd);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "webpage_clear_dirty_flag failed: %s\n",
				PQerrorMessage(conn));
		PQclear(res);
		return -1;
	}
	PQclear(res);
	return 0;
}


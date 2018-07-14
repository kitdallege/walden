#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "flag_flipper.h"

FlagFlipperState *flag_flipper_new(void)
{
	FlagFlipperState *st = malloc(sizeof(*st));
	Controller *ctl = malloc(sizeof(*ctl));
	controller_init(ctl);
	st->ctl = ctl;
	st->wq = bqueue_new();
	return st;
}

PageIdArray *page_id_array_create(size_t len)
{
	PageIdArray *arr = malloc(sizeof(*arr) + (sizeof(int) * len));
	arr->len = 0;
	for (unsigned int i=0; i < len; i++){arr->data[i] = 0;}
	return arr;
}

/* return'd char * is a comma separated list of the int arr[] arg..
 * ex: "#,#,#,#"
 * note: no trailing comma. also no comma on single element lists.
 */
static char *array_to_str(unsigned int arr[], size_t len)
{
	int d, num, d_idx = 0;
	char temp[21] = { 0 };
	char *p, *bp, *buf = calloc(20 * len + 4, sizeof(*buf));
	bp = buf;
	*bp++ = '{';
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
	*bp++ = '}';
	*bp = '\0';
	return buf;
}

void *webpage_clear_dirty_thread(void *arg)
{
	FlagFlipperState *st = (FlagFlipperState*)arg;
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
	// setup prepared statement
	res = PQprepare(conn, "flip-flag",
		"update webpage set date_updated = default, dirty = false where id = ANY($1);",
		1, NULL);
	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		fprintf(stderr, "failed to prepare statement!\n");
		PQclear(res);
	}
	PQclear(res);
	// build query out of passed in ids.
	PageIdArray *work;
	char *arr_str;
	const char *vals[1];

	while (st->ctl->active) {
		while (bqueue_empty(st->wq) && st->ctl->active) {
			//fprintf(stderr, "empty: %d size: %lu, active: %d : \n", bqueue_empty(st->wq), bqueue_size(st->wq), st->ctl->active);
			pthread_cond_wait(&st->ctl->cond, &st->ctl->mutex);
		}
		//fprintf(stderr, "empty: %d size: %lu, active: %d : \n", bqueue_empty(st->wq), bqueue_size(st->wq), st->ctl->active);
		if (!st->ctl->active) { break; }
		if (bqueue_empty(st->wq)) { continue;}
		work = bqueue_pop(st->wq); // peek ? would move this to the conditional
		pthread_mutex_unlock(&st->ctl->mutex);

		// build & run query 
		arr_str = array_to_str(work->data, work->len);
		vals[0] = arr_str; 
		//fprintf(stderr, "ids: %s\n", vals[0]);
		//fprintf(stderr, "updating %lu page(s).\n", work->len);
		res = PQexecPrepared(conn, "flip-flag", 1, vals, NULL, NULL, 0);
		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			fprintf(stderr, "webpage_clear_dirty_flag failed: %s\n", PQerrorMessage(conn));
		}
		free(work);
		free(arr_str);
		PQclear(res);
		pthread_mutex_lock(&st->ctl->mutex);
	}
	fprintf(stderr, "empty: %d size: %lu, active: %d : \n", bqueue_empty(st->wq), bqueue_size(st->wq), st->ctl->active);
	fprintf(stderr, "flag flipper shutting down..\n");
	fprintf(stderr, "size: %lu\n", bqueue_size(st->wq));
	//free(cmd);
	bqueue_del(st->wq);
	controller_destory(st->ctl); free(st->ctl);
	free(st);
	fprintf(stderr, "flag fliper exiting\n");
	PQfinish(conn);
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


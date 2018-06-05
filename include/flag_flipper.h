#ifndef FLAG_FLIPPER_H 
#define FLAG_FLIPPER_H

#include <libpq-fe.h>

#include "queue.h"
#include "controller.h"

typedef struct flag_flipper_state {
	controller ctl;
	queue wq;
} flag_flipper_state;

flag_flipper_state *flag_flipper_new(void);
void *webpage_clear_dirty_thread(void *arg);
int webpage_clear_dirty_flag(PGconn *conn, int id);

#endif


#ifndef FLAG_FLIPPER_H 
#define FLAG_FLIPPER_H

#include <libpq-fe.h>

#include "bqueue.h"
#include "controller.h"

typedef struct FlagFlipperState {
	Controller *ctl;
	BQueue *wq;
} FlagFlipperState;

typedef struct PageIdArray {
	unsigned int *data;
	size_t len;
} PageIdArray;

FlagFlipperState *flag_flipper_new(void);
void *webpage_clear_dirty_thread(void *arg);
int webpage_clear_dirty_flag(PGconn *conn, int id);

PageIdArray *page_id_array_create(size_t len);
#endif


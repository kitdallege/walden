#ifndef HANDLERS_H
#define HANDLERS_H

typedef struct Handler Handler;	
typedef enum FileEventType
{
	FET_ADD,
	FET_MOD,
	FET_DEL,
	FET_COUNT
} FileEventType;
typedef struct FileEvent
{
	char *filename;
	enum FileEventType type;
} FileEvent;

// life cycle
Handler	   *handler_aloc(void);
void		handler_conf(Handler *self, void *user);
void		handler_zero(Handler *self);
void		handler_free(Handler *self);
// api
void		handler_step(Handler *self, void *user);
void		handler_enqueue_event(Handler *self, FileEvent *event);
void		handler_sync_all(Handler *self);
#endif

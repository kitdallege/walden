#ifndef HANDLERS_H
#define HANDLERS_H

typedef struct Handler Handler;	

// life cycle
Handler	   *handler_aloc(void);
void		handler_conf(Handler *self, void *user);
void		handler_zero(Handler *self);
void		handler_free(Handler *self);

// api
void		handler_step(Handler *self, void *user);

// this becomes step
//void		handler_process_event(Handler *self, InotifyEvent *evt);

#endif

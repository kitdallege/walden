#ifndef CONTROLLER_H 
#define CONTROLLER_H

typedef struct Controller {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int active;
} Controller;

int controller_init(Controller *c);
int controller_destory(Controller *c);
int controller_activate(Controller *c);
int controller_deactivate(Controller *c);
int controller_force(Controller *c);
int controller_unforce(Controller *c);

#endif


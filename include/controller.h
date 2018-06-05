#ifndef CONTROLLER_H 
#define CONTROLLER_H

typedef struct controller {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int active;
	int force;
} controller;

int controller_init(controller *c);
int controller_destory(controller *c);
int controller_activate(controller *c);
int controller_deactivate(controller *c);
int controller_force(controller *c);
int controller_unforce(controller *c);

#endif


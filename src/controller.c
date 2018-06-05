#include <stdio.h>
#include <pthread.h>

#include "controller.h"


int controller_init(controller *c)
{
	if (pthread_mutex_init(&(c->mutex), NULL) ||
		pthread_cond_init(&(c->cond), NULL)) {
		return 1;
	}
	c->active = 0;
	c->force = 0;
	return 0;
}

int controller_destory(controller *c)
{
	if (pthread_cond_destroy(&(c->cond)) ||
		pthread_mutex_destroy(&(c->mutex))) {
		return 1;
	}
	c->active = 0;
	c->force = 0;
	return 0;
}

int controller_activate(controller *c)
{
	if (pthread_mutex_lock(&(c->mutex))) {
		return 0;
	}
	c->active = 1;
	pthread_mutex_unlock(&(c->mutex));
	pthread_cond_broadcast(&(c->cond));
	return 1;
}

int controller_deactivate(controller *c)
{
	if (pthread_mutex_lock(&(c->mutex))) {
		return 0;
	}
	c->active = 0;
	pthread_mutex_unlock(&(c->mutex));
	pthread_cond_broadcast(&(c->cond));
	return 1;
}

int controller_force(controller *c)
{
	if (pthread_mutex_lock(&(c->mutex))) {
		return 0;
	}
	c->force = 1;
	pthread_mutex_unlock(&(c->mutex));
	pthread_cond_broadcast(&(c->cond));
	return 1;
}

int controller_unforce(controller *c)
{
	fprintf(stderr, "unforce lock mutex\n");
	if (pthread_mutex_lock(&(c->mutex))) {
		return 0;
	}
	c->force = 0;
	fprintf(stderr, "unforce unlockng mutex\n");
	pthread_mutex_unlock(&(c->mutex));
	fprintf(stderr, "unforce unlocked mutex\n");
	pthread_cond_broadcast(&(c->cond));
	return 1;
}


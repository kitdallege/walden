#ifndef QUEUE_H 
#define QUEUE_H

//typedef void * queue_item;
typedef unsigned int queue_item;

// TODO: node could just contain *next and then be used 'in' a userland
// struct so that the value can be stored instead of *pointed to.
typedef struct node {
	struct node *next;
	queue_item value;
} node;

typedef struct queue {
	node *head, *tail;
	size_t size;
} queue;


queue *queue_new(void);
void queue_put(queue *q, queue_item item);
queue_item queue_get(queue *q);

#endif

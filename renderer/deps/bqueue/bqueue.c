#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bqueue.h"

#define BLOCK_QUEUE_IPB 100

// Lifecycle
BQueue *bqueue_new()
{
	BQueue *q = malloc(sizeof(*q));
	q->size = 0;
	q->items_per_block = BLOCK_QUEUE_IPB;
	q->head_idx = q->tail_idx = 0;
	Block *blk = malloc(sizeof(*blk));
	blk->data = malloc(sizeof(blk->data) * BLOCK_QUEUE_IPB);
	blk->next = NULL;
	q->head = q->tail = blk;
	return q;
}

void bqueue_del(BQueue *queue)
{
	Block *temp, *b = queue->head;
	while (b) {
		temp = b;
		b = b->next;
		free(temp->data);
		free(temp);
	}
	free(queue);
}

// Public API 
int bqueue_push(BQueue *queue, BQItem item)
{
	if (queue->tail_idx == queue->items_per_block) {
		Block *blk = malloc(sizeof(*blk));
		blk->data = malloc(sizeof(blk->data) * BLOCK_QUEUE_IPB);
		blk->next = NULL;
		queue->tail->next = blk;
		queue->tail = blk;
		queue->tail_idx = 0;
	}
	queue->tail->data[queue->tail_idx] = item;
	queue->tail_idx++;
	queue->size++;
	return 0;
}

BQItem bqueue_pop(BQueue *queue)
{
	if (bqueue_empty(queue)) {
		return NULL;
	}
	if (queue->head_idx == queue->items_per_block) {
		Block *blk = queue->head;
		queue->head = blk->next;
		queue->head_idx = 0;
		free(blk->data);
		free(blk);
	}
	BQItem item = bqueue_front(queue);
	queue->head_idx++;
	queue->size--;
	return item; 
}
// TODO: inline (or) macro
bool bqueue_empty(BQueue *queue)
{
	return queue->size == 0;
}
// TODO: inline (or) macro
size_t bqueue_size(BQueue *queue)
{
		return queue->size;
}

BQItem bqueue_front(BQueue *queue)
{
	return queue->head->data[queue->head_idx];
}

BQItem bqueue_back(BQueue *q)
{
	return q->tail->data[(q->tail_idx ? q->tail_idx - 1 : 0)];
}


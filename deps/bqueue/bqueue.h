#ifndef BQUEUE_H 
#define BQUEUE_H

#include <stdint.h>
#include <stdbool.h>

/**************************************************************************** 
 * BlockQueue idea based on a cross of Python's deque and a 'bip buffer'
 * https://www.codeproject.com/Articles/3479/The-Bip-Buffer-The-Circular-Buffer-with-a-Twist
 * https://github.com/python/cpython/blob/master/Modules/_collectionsmodule.c
 *
 * Internally the queue is a mix of contigious array & linked list.
 * The queue uses a linked list of block(s) as its internal storage.
 * A minium of 2 blocks are always in play, but can grow as required.
 * Items are push'd onto the tail and pop'd from the head.
 * The head and tail can reside on different blocks.
 ****************************************************************************/
typedef void* BQItem;

typedef struct Block {
	struct Block *next; 
	unsigned char data[];
} Block;

typedef struct BQueue {
	size_t item_size, size;
	Block *head, *tail;
	int head_idx, tail_idx, items_per_block;
} BQueue;

typedef struct BQOpts {
	size_t item_size;    	
} BQOpts;

// Lifecycle
BQueue *bqueue_new(BQOpts opts);
void bqueue_del(BQueue *queue);

// Public API  
int bqueue_push(BQueue *queue, BQItem item);
BQItem bqueue_pop(BQueue *queue);
bool bqueue_empty(BQueue *queue);
size_t bqueue_size(BQueue *queue);
BQItem bqueue_front(BQueue *queue);
BQItem bqueue_back(BQueue *queue);

#endif

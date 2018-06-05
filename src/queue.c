#include <stdlib.h>

#include "queue.h"


queue *queue_new(void)
{
	queue *q = malloc(sizeof(*q));
	q->head = q->tail = NULL;
	q->size = 0;
	return q;
}

void queue_put(queue *q, queue_item item)
{
	q->size++;
	node *n = malloc(sizeof(*n));
	*n = (node){.next = NULL, .value = item};
	if (!q->tail) {
		q->head = q->tail= n;
		return ;
	}
	q->tail->next = q->tail = n;
}

queue_item queue_get(queue *q)
{
	if (!q->head) {
		return NULL;
	}
	q->size--;
	node *n = q->head;
	q->head = q->head->next;
	if (!q->head) {
		q->tail = NULL;
	}
	queue_item val = n->value;
	free(n);
	return val;
}

/*
 ==1097== Invalid write of size 8                                                                                                                                                             
==1097==    at 0x10AF56: queue_put (queue.c:18)                                                                                                                                              
==1097==    by 0x10A9AA: handle_page (renderer.c:90)                                                                                                                                         
==1097==    by 0x10B51D: main (main.c:129)                                                                                                                                                   
==1097==  Address 0x8f32430 is 0 bytes inside a block of size 16 free'd                                                                                                                      
==1097==    at 0x4C2CDDB: free (vg_replace_malloc.c:530)                                                                                                                                     
==1097==    by 0x10AFCA: queue_get (queue.c:32)                                                                                                                                              
==1097==    by 0x10AD83: webpage_clear_dirty_thread (flag_flipper.c:25)                                                                                                                      
==1097==    by 0x56B5493: start_thread (pthread_create.c:333)                                                                                                                                
==1097==    by 0x59B3ACE: clone (clone.S:97)                                                                                                                                                 
==1097==  Block was alloc'd at                                                                                                                                                               
==1097==    at 0x4C2BBAF: malloc (vg_replace_malloc.c:299)                                                                                                                                   
==1097==    by 0x10AF23: queue_put (queue.c:15)                                                                                                                                              
==1097==   by 0x10A9AA: handle_page (renderer.c:90)                                                                                                                                         
==1097==   
 */

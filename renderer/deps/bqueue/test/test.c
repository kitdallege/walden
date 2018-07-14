#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "bqueue.h"
#define TEST_ARRAY_LEN 1000
// test defs
void test_queue_new(void);
void test_queue_push(void);
void test_queue_pop(void);
void test_queue_back(void);
void test_queue_front(void);
void test_scope_outer(void);

int main(int argc, char **argv)
{
	fprintf(stdout, "test_queue_new: \n");
	test_queue_new();
	fprintf(stdout, "test_queue_push: \n");
	test_queue_push();
	fprintf(stdout, "test_queue_pop: \n");
	test_queue_pop();
	fprintf(stdout, "test_queue_back: \n");
	test_queue_back();
	fprintf(stdout, "test_queue_front: \n");
	test_queue_front();
	fprintf(stdout, "test_scope_outer: \n");
	test_scope_outer();
	fprintf(stdout, "\n  \033[32m\u2713 \033[90mok\033[0m\n\n");
}

/*
 * Tests
 */
void test_queue_new()
{
	BQueue *q = bqueue_new();
	assert(bqueue_size(q) == 0);
	assert(bqueue_empty(q) == true);
	bqueue_del(q);
}

void test_queue_push()
{
	BQueue *q = bqueue_new();
	assert(bqueue_size(q) == 0);
	for (int i = 0; i < TEST_ARRAY_LEN; i++) {
		bqueue_push(q, &i);
	}
	assert(bqueue_size(q) == TEST_ARRAY_LEN);
	bqueue_del(q);
}

void test_queue_pop()
{
	BQueue *q = bqueue_new();
	assert(bqueue_size(q) == 0);
	int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
	for (int i = 0; i < 20; i++) {
		bqueue_push(q, &arr[i]);
	}
//	assert(bqueue_size(q) == TEST_ARRAY_LEN);
	for (int i =0; i < TEST_ARRAY_LEN; i++) {
		bqueue_pop(q);
	}
	assert(bqueue_size(q) == 0);
	assert(bqueue_empty(q) == true);
	bqueue_del(q);
}

void test_queue_back()
{
	BQueue *q = bqueue_new();
	assert(bqueue_size(q) == 0);
	int x = 10;
	bqueue_push(q, &x);
	void *tail = bqueue_back(q);
	assert(*(int *)tail == x);
	assert(bqueue_size(q) == 1);
	bqueue_del(q);
}

void test_queue_front()
{
	BQueue *q = bqueue_new();
	assert(bqueue_size(q) == 0);
	int x = 10;
	bqueue_push(q, &x);
	void *head = bqueue_front(q);
	assert(*(int *)head == x);
	assert(bqueue_size(q) == 1);
	bqueue_del(q);
}

void test_scope_outer()
{
	BQItem val;
	BQueue *q = bqueue_new();
	unsigned int *ptr = malloc(sizeof(*ptr) * TEST_ARRAY_LEN);
	for (unsigned int i=0; i < TEST_ARRAY_LEN; i++) { ptr[i] = i+1; }	   
	for (int t=0; t< 10; t++) {
		for (unsigned int i=0; i < TEST_ARRAY_LEN; i++) {
			bqueue_push(q, &ptr[i]);
			//fprintf(stderr, "bqueue_push: %u (%p) size: %lu\n", ptr[i], (void *)&ptr[i], bqueue_size(q));
		}
		fprintf(stderr, "run # %d - queue size: %lu\n", t, bqueue_size(q));
		for (int i=0; i < TEST_ARRAY_LEN; i++) {
			val = bqueue_pop(q);
			if (*(unsigned int *)val > 1010) {
				fprintf(stderr, "bqueue_pop: %u (%p) size: %lu -- test \n", *(unsigned int *)val, val, bqueue_size(q));
			}
		}
	}
	free(ptr);
	bqueue_del(q);
}

void scope_inner(void);
void scope_inner()
{

}

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "bqueue.h"
#define TEST_ARRAY_LEN 200
// test defs
void test_queue_new(void);
void test_queue_push(void);
void test_queue_pop(void);
void test_queue_back(void);
void test_queue_front(void);

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
	fprintf(stdout, "\n  \033[32m\u2713 \033[90mok\033[0m\n\n");
}

/*
 * Tests
 */
void test_queue_new()
{
	BQueue *q = bqueue_new((BQOpts){.item_size = sizeof(int)});
	assert(bqueue_size(q) == 0);
	assert(bqueue_empty(q) == true);
	bqueue_del(q);
}

void test_queue_push()
{
	BQueue *q = bqueue_new((BQOpts){.item_size = sizeof(int)});
	assert(bqueue_size(q) == 0);
	for (int i = 0; i < TEST_ARRAY_LEN; i++) {
		bqueue_push(q, &i);
	}
	assert(bqueue_size(q) == TEST_ARRAY_LEN);
	bqueue_del(q);
}

void test_queue_pop()
{
	BQueue *q = bqueue_new((BQOpts){.item_size = sizeof(int *)});
	assert(bqueue_size(q) == 0);
	int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
	for (int i = 0; i < TEST_ARRAY_LEN; i++) {
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
	BQueue *q = bqueue_new((BQOpts){.item_size = sizeof(int)});
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
	BQueue *q = bqueue_new((BQOpts){.item_size = sizeof(int)});
	assert(bqueue_size(q) == 0);
	int x = 10;
	bqueue_push(q, &x);
	void *head = bqueue_front(q);
	assert(*(int *)head == x);
	assert(bqueue_size(q) == 1);
	bqueue_del(q);
}


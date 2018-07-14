
CC = gcc

CFLAGS += -g -O0 -Wall -Wstrict-prototypes -Wmissing-prototypes -Wpadded
CFLAGS  += -Wmissing-declarations -Wshadow -Wpointer-arith -Wno-unused-function
CFLAGS  += -Wsign-compare -std=c99 -pedantic -I./


test/test: bqueue.c test/test.c
	$(CC) $^ $(CFLAGS) -o $@

clean:
	@rm test/test

.PHONY: test clean


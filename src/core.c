#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>

#include "core.h"

static Memory memory = {0};
int top = 0;
uint64_t total_storage_size = 0;
void *base_address = (void *)Terabytes(4);	


void memory_init(void)
{
	memory.persistent_storage_size = Megabytes(2);
	memory.transient_storage_size = Megabytes(2);
	total_storage_size = memory.persistent_storage_size + memory.transient_storage_size;
	memory.persistent_storage = mmap(
		base_address, total_storage_size,
		PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE,
		-1, 0
	);
	memory.transient_storage = (uint8_t*)(memory.persistent_storage) + memory.persistent_storage_size;
}

void memory_free(void)
{
	munmap(base_address, total_storage_size);
}

void *acquire(size_t bytes)
{
	void *ptr;
    ptr = (char *)memory.persistent_storage;
	top += bytes;
	fprintf(stderr, "acquire: %p\n", memory.persistent_storage);
	fprintf(stderr, "next: %p\n", (char *)memory.persistent_storage + top);
	return ptr;
}

void free(void *ptr)
{

}



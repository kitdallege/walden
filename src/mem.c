#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <malloc.h>

#include "mem.h"

struct Memory
{
	size_t persistent_storage_size;
	void  *persistent_storage;
	size_t transient_storage_size;
	void  *transient_storage;
};

static Memory memory = {0};
int top = 0;
int top_p = 0, top_t = 0;
uint64_t total_storage_size = 0;
void *base_address = (void *)Terabytes(4);	

// current api
void memory_init(void)
{
	mem_system_acquire();
}
void memory_free(void)
{
	mem_system_release();
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

// future api
void mem_system_acquire(void)
{
	memory.persistent_storage_size = Kilobytes(2);
	memory.transient_storage_size = Kilobytes(56);
	total_storage_size = memory.persistent_storage_size + memory.transient_storage_size;
	memory.persistent_storage = mmap(
		base_address, total_storage_size,
		PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE,
		-1, 0
	);
	memory.transient_storage = (uint8_t*)(memory.persistent_storage) + memory.persistent_storage_size;
	fprintf(stderr, "memory.persistent_storage: %p\n", memory.persistent_storage);
	fprintf(stderr, "memory.transient_storage: %p\n", memory.transient_storage);
}

void mem_system_release(void)
{
	fprintf(stderr, "void mem_system_release: start: %p  end: %p\n", base_address, ((char *)base_address + total_storage_size));
	munmap(base_address, total_storage_size);
}

void *mem_acquire_p(size_t bytes)
{
	if (top_p + bytes > memory.persistent_storage_size) {
		fprintf(stderr, "No persistent_storage memory left.\n");
		return NULL;
	}
	void *ptr;
    ptr = (char *)memory.persistent_storage + top_p;
	top_p += bytes;
	fprintf(stderr, "acquire: %p\n", ptr);
	fprintf(stderr, "next: %p\n", (char *)memory.persistent_storage + top_p);
	return ptr;

}

void *mem_acquire(size_t bytes)
{
	// todo create a 'freelist algo' 
	if (top_t + bytes > memory.transient_storage_size) {
		fprintf(stderr, "No transient_storage memory left.\n");
		return NULL;
	}
	void *ptr;
    ptr = (char *)memory.transient_storage + top_t;
	top_t += bytes;
	fprintf(stderr, "mem_acquire: %p\n", ptr);
	fprintf(stderr, "next: %p\n", (char *)memory.transient_storage + top_t);
	return ptr;
	
}

void  mem_release(void *ptr)
{
	fprintf(stderr, "mem_release: %p\n", ptr);
	// mark as a free block ?
}

static void display_mallinfo(FILE *stream)
{
    struct mallinfo mi;
    mi = mallinfo();
    fprintf(stream, "Total non-mmapped bytes (arena):       %d\n", mi.arena);
    fprintf(stream, "# of free chunks (ordblks):            %d\n", mi.ordblks);
    fprintf(stream, "# of free fastbin blocks (smblks):     %d\n", mi.smblks);
    fprintf(stream, "# of mapped regions (hblks):           %d\n", mi.hblks);
    fprintf(stream, "Bytes in mapped regions (hblkhd):      %d\n", mi.hblkhd);
    fprintf(stream, "Max. total allocated space (usmblks):  %d\n", mi.usmblks);
    fprintf(stream, "Free bytes held in fastbins (fsmblks): %d\n", mi.fsmblks);
    fprintf(stream, "Total allocated space (uordblks):      %d\n", mi.uordblks);
    fprintf(stream, "Total free space (fordblks):           %d\n", mi.fordblks);
    fprintf(stream, "Topmost releasable block (keepcost):   %d\n", mi.keepcost);
}

void mem_write_report(FILE *stream)
{
	fprintf(stream, "\n ------------------------------------ \n");
	fprintf(stream, "\t Memory Statisics \n");
	fprintf(stream, "persistent_storage_size: %lu bytes\n", memory.persistent_storage_size);
	fprintf(stream, "transient_storage_size:  %lu bytes\n", memory.transient_storage_size);
	fprintf(stream, "total_storage_size:      %lu bytes\n", total_storage_size);
	fprintf(stream, "persistent_storage used: %d bytes\n", top_p);
	fprintf(stream, "persistent_storage free: %lu bytes\n", memory.persistent_storage_size - top_p);
	fprintf(stream, "transient_storage used:  %d bytes\n", top_t);
	fprintf(stream, "transient_storage free:  %lu bytes\n", memory.transient_storage_size - top_t);
	fprintf(stream, "\n ------------------------------------ \n");
	fprintf(stream, "Malloc info: \n");
	display_mallinfo(stream);
	fprintf(stream, "\n ------------------------------------ \n");
}


#ifndef MEM_H
#define MEM_H

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

typedef struct Memory
{
	size_t persistent_storage_size;
	void  *persistent_storage;
	size_t transient_storage_size;
	void  *transient_storage;
} Memory;

void memory_init(void);
void memory_free(void);

void *acquire(size_t bytes);
void free(void *ptr);
#endif


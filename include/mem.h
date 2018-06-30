#ifndef MEM_H
#define MEM_H

#include <stdio.h>

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

typedef struct Memory Memory;

// application wide memory lifecycle
void  mem_system_acquire(void); 
void  mem_system_release(void);
// memory management functions
void *mem_acquire_p(size_t bytes); 	// works on persistent_storage *note: no release*
void *mem_acquire(size_t bytes);   	// operates on transient_storage
void  mem_release(void *ptr); 		// operates on transient_storage

// arean/pool stuff.
void  mem_write_report(FILE *stream); 	// writes to stderr a report of stats.
#endif


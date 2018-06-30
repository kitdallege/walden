#ifndef MEM_H
#define MEM_H

#include <stdio.h>

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

typedef struct Memory Memory;

// current api
void memory_init(void); // mem_system_acquire 
void memory_free(void); // mem_system_release
void *acquire(size_t bytes);
void free(void *ptr);

// future api
//
// application wide memory lifecycle
void mem_system_acquire(void); 
void mem_system_release(void);
// memory management functions
void *mem_acquire_p(size_t bytes);	// works on persistent_storage *note: no release*
void *mem_acquire(size_t bytes);	// operates on transient_storage
void  mem_release(void *ptr);		// operates on transient_storage

// stats
/*
 * Need to keep basic stats on memory, should allow me to shink the apps
 * footprint to 'just what it needs'.
 *
 * Also keep an eye on fragmentation of transient_storage
 * used / in-use 
 * allocs / frees
 *
 */
void mem_write_report(FILE *stream); // writes to stderr a report of stats.
// (void) write_memory_report(stderr);
#endif


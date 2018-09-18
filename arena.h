#ifndef ARENA_H_
#define ARENA_H_

#include <inttypes.h>   // for PRIx64
#include <stdint.h>     // for uint64_t
#include <stdio.h>      // for FILE*
#include "cell.h"       // for struct Cell

// basically, 64 bits
#define ARENA_POOL_SIZE (8*sizeof(uint64_t))

// a 1 bit in the pool mask indicates a free slot
#define POOL_EMPTY      UINT64_MAX
#define POOL_MASK_FMT   PRIx64

#define POOL_IS_USED(m, x) \
    (!((m) & (1ULL << x)))
#define POOL_MARK_USED(m, x) \
    do { (m) &= ~(1ULL << x); } while (0)
#define POOL_MARK_FREE(m, x) \
    do { (m) |=  (1ULL << x); } while (0)

typedef struct Pool {
    Cell slots[ARENA_POOL_SIZE];  // each pool has this many cells
    uint64_t mask;                // keep track of used slots
    struct Pool* next;            // link to next pool
} Pool;

typedef struct Arena {
    int size;      // how many pools we have
    Pool* cells;   // linked list of pools
} Arena;

Arena* arena_create(void);
void arena_destroy(Arena* arena);

// get an "empty" cell from the arena, as if created with malloc
Cell* arena_get_cell(Arena* arena, int hint);

// set all the cells in the arena to empty
void arena_reset_to_empty(Arena* arena);

int arena_cell_used(Arena* arena, const Cell* cell);

// mark a specific cell in the arena as used
void arena_mark_cell_used(Arena* arena, const Cell* cell, const char* name);

// get the pool in the arena where a specific cell lives
Pool* arena_pool_for_cell(Arena* arena, const Cell* cell);

// print arena stats to a file
void arena_dump(Arena* arena, FILE* fp);

#endif

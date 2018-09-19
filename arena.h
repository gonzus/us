#ifndef ARENA_H_
#define ARENA_H_

#include <inttypes.h>   // for PRIx64
#include <stdint.h>     // for uint64_t
#include <stdio.h>      // for FILE*
#include "cell.h"       // for struct Cell
#include "env.h"        // for struct Env

// basically, 64 bits
#define ARENA_POOL_SIZE (8*sizeof(uint64_t))

// a 1 bit in the pool mask indicates a free slot
#define POOL_EMPTY      UINT64_MAX
#define POOL_MASK_FMT   PRIx64

#define POOL_IS_USED(m, x)    (!((m) & (1ULL << x)))
#define POOL_MARK_USED(m, x)  do { (m) &= ~(1ULL << x); } while (0)
#define POOL_MARK_FREE(m, x)  do { (m) |=  (1ULL << x); } while (0)

typedef struct CellPool {
    Cell slots[ARENA_POOL_SIZE];  // each pool has this many cells
    uint64_t mask;                // keep track of used slots
    struct CellPool* next;        // link to next pool
} CellPool;

typedef struct EnvPool {
    Env slots[ARENA_POOL_SIZE];   // each pool has this many envs
    uint64_t mask;                // keep track of used slots
    struct EnvPool* next;         // link to next pool
} EnvPool;

typedef struct Arena {
    CellPool* cells;    // linked list of cell pools
    EnvPool* envs;      // linked list of env pools
} Arena;

Arena* arena_create(void);
void arena_destroy(Arena* arena);

// get an "empty" cell/env from the arena, as if created with malloc
Cell* arena_get_cell(Arena* arena, int hint);
Env* arena_get_env(Arena* arena, int hint);

// set all the cells/envs in the arena to "not used"
void arena_reset_to_empty(Arena* arena);

// check whether a cell/env is currently marked as used
int arena_is_cell_used(Arena* arena, const Cell* cell);
int arena_is_env_used(Arena* arena, const Env* env);

// mark a specific cell/env in the arena as used
void arena_mark_cell_used(Arena* arena, const Cell* cell);
void arena_mark_env_used(Arena* arena, const Env* env);

// get the pool in the arena where a specific cell/env lives
CellPool* arena_get_pool_for_cell(Arena* arena, const Cell* cell);
EnvPool* arena_get_pool_for_env(Arena* arena, const Env* env);

// print arena stats to a file
void arena_dump(Arena* arena, FILE* fp);

#endif

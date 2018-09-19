#include <stdlib.h>
#include <strings.h>
#include "arena.h"

#if !defined(MEM_DEBUG)
#define MEM_DEBUG 0
#endif
#include "mem.h"

// #define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"

Arena* arena_create(void)
{
    Arena* arena = 0;
    MEM_ALLOC_TYPE(arena, 1, Arena);
    LOG(INFO, ("arena: created %p", arena));
    return arena;
}

static void cell_cleanup(Cell* cell)
{
    switch (cell->tag) {
        case CELL_STRING:
        case CELL_SYMBOL:
            MEM_FREE_SIZE(cell->sval, 0);
            break;
    }
    cell->tag = CELL_NONE;
}

static void env_cleanup(Env* env)
{
    env->parent = 0;
    for (int j = 0; j < env->size; ++j) {
        for (Symbol* sym = env->table[j]; sym; ) {
            Symbol* tmp = sym;
            sym = sym->next;
            MEM_FREE_SIZE(tmp->name, 0);
            MEM_FREE_TYPE(tmp, 1, Symbol);
        }
        env->table[j] = 0;
    }
}

void arena_destroy(Arena* arena)
{
    int count = 0;
    LOG(INFO, ("arena: destroying arena %p", arena));
    count = 0;
    for (CellPool* pool = arena->cells; pool; ) {
        for (unsigned long j = 0; j < ARENA_POOL_SIZE; ++j) {
            cell_cleanup(&pool->slots[j]);
        }
        CellPool* tmp = pool;
        pool = pool->next;
        LOG(DEBUG, ("arena: destroying cell pool %p", tmp));
        MEM_FREE_TYPE(tmp, 1, CellPool);
        ++count;
    }
    LOG(INFO, ("arena: destroyed %d cell pools, %lu bytes", count, (unsigned long) (count * sizeof(CellPool))));

    count = 0;
    for (EnvPool* pool = arena->envs; pool; ) {
        for (unsigned long j = 0; j < ARENA_POOL_SIZE; ++j) {
            Env* env = &pool->slots[j];
            env_cleanup(env);
            MEM_FREE_TYPE(env->table, env->size, Symbol*);
        }
        EnvPool* tmp = pool;
        pool = pool->next;
        LOG(DEBUG, ("arena: destroying env pool %p", tmp));
        MEM_FREE_TYPE(tmp, 1, EnvPool);
        ++count;
    }
    LOG(INFO, ("arena: destroyed %d env pools, %lu bytes", count, (unsigned long) (count * sizeof(EnvPool))));

    MEM_FREE_TYPE(arena, 1, Arena);
}

// NOTE: from man page for fls:
// The ffs(), ffsl() and ffsll() functions find the first (least significant)
// bit set in value and return the index of that bit.  Bits are numbered
// starting at 1, the least significant bit.  A return value of zero from any
// of these functions means that the argument was zero.

Cell* arena_get_cell(Arena* arena, int hint)
{
    (void) hint;
    CellPool* pool = 0;
    int pos = 0;
    for (pool = arena->cells; pool; pool = pool->next) {
        // find first unused slot in this pool
        int lsb = ffsll(pool->mask);
        if (lsb) {
            // there is at least one bit set to 1 => free slot
            pos = lsb - 1;
            break;
        }
    }

    if (!pool) {
        // Need to create a new cell pool
        MEM_ALLOC_TYPE(pool, 1, CellPool);
        LOG(DEBUG, ("arena: created cell pool %p", pool));
        pool->mask = POOL_EMPTY;
        pool->next = arena->cells;
        arena->cells = pool;
        pos = 0;
    }

    // mark pos as used to return it
    POOL_MARK_USED(pool->mask, pos);

    // free any data that might still be in the cell
    Cell* cell = &pool->slots[pos];
    cell_cleanup(cell);
    return cell;
}

Env* arena_get_env(Arena* arena, int hint)
{
    EnvPool* pool = 0;
    int pos = 0;
    for (pool = arena->envs; pool; pool = pool->next) {
        // find first unused slot in this pool
        int lsb = ffsll(pool->mask);
        if (lsb) {
            // there is at least one bit set to 1 => free slot
            pos = lsb - 1;
            break;
        }
    }

    if (!pool) {
        // Need to create a new pool
        MEM_ALLOC_TYPE(pool, 1, EnvPool);
        LOG(DEBUG, ("arena: created env pool %p", pool));
        pool->mask = POOL_EMPTY;
        pool->next = arena->envs;
        arena->envs = pool;
        pos = 0;
    }

    // mark pos as used and return it
    POOL_MARK_USED(pool->mask, pos);

    Env* env = &pool->slots[pos];
    env_cleanup(env);
    if (env->size) {
        LOG(DEBUG, ("ARENA - ENV: reusing %p, %d buckets at %p", env, env->size, env->table));
    } else {
        env->size = hint ? hint : 1021;
        MEM_ALLOC_TYPE(env->table, env->size, Symbol*);
        LOG(DEBUG, ("ARENA - ENV: created %p, %d buckets at %p", env, env->size, env->table));
    }
    return env;
}

void arena_reset_to_empty(Arena* arena)
{
    for (CellPool* pool = arena->cells; pool; pool = pool->next) {
        pool->mask = POOL_EMPTY;
    }
    for (EnvPool* pool = arena->envs; pool; pool = pool->next) {
        pool->mask = POOL_EMPTY;
    }
}

int arena_is_cell_used(Arena* arena, const Cell* cell)
{
    CellPool* pool = arena_get_pool_for_cell(arena, cell);
    if (!pool) {
        return 0;
    }
    int pos = cell - pool->slots;
    return POOL_IS_USED(pool->mask, pos);
}

int arena_is_env_used(Arena* arena, const Env* env)
{
    EnvPool* pool = arena_get_pool_for_env(arena, env);
    if (!pool) {
        return 0;
    }
    int pos = env - pool->slots;
    return POOL_IS_USED(pool->mask, pos);
}

void arena_mark_cell_used(Arena* arena, const Cell* cell)
{
    CellPool* pool = arena_get_pool_for_cell(arena, cell);
    if (!pool) {
        return;
    }
    int pos = cell - pool->slots;
    POOL_MARK_USED(pool->mask, pos);
}

void arena_mark_env_used(Arena* arena, const Env* env)
{
    EnvPool* pool = arena_get_pool_for_env(arena, env);
    if (!pool) {
        return;
    }
    int pos = env - pool->slots;
    POOL_MARK_USED(pool->mask, pos);
}

CellPool* arena_get_pool_for_cell(Arena* arena, const Cell* cell)
{
    for (CellPool* pool = arena->cells; pool; pool = pool->next) {
        if (pool->slots <= cell && cell < (pool->slots + ARENA_POOL_SIZE)) {
            return pool;
        }

    }
    LOG(DEBUG, ("ARENA: cell %p out of bound -- WTF?", cell));
    return 0;
}

EnvPool* arena_get_pool_for_env(Arena* arena, const Env* env)
{
    for (EnvPool* pool = arena->envs; pool; pool = pool->next) {
        if (pool->slots <= env && env < (pool->slots + ARENA_POOL_SIZE)) {
            return pool;
        }

    }
    LOG(DEBUG, ("ARENA: env %p out of bound -- WTF?", env));
    return 0;
}

void arena_dump(Arena* arena, FILE* fp)
{
    int count = 0;
    fprintf(fp, "=== Arena at %p\n", arena);

    count = 0;
    for (CellPool* pool = arena->cells; pool; pool = pool->next) {
        int free = 0;
        for (unsigned long j = 0; j < ARENA_POOL_SIZE; ++j) {
            int b = 1ULL << j;
            if (pool->mask & b) {
                ++free;
            }
        }
        ++count;
        // fprintf(fp, "===  CellPool at %p, free slots %d, mask %0*" PRIx64 "\n", pool, free, (int) (ARENA_POOL_SIZE * 2 / sizeof(uint64_t)), pool->mask);
    }
    fprintf(fp, "===  CellPool count: %d\n", count);

    count = 0;
    for (EnvPool* pool = arena->envs; pool; pool = pool->next) {
        int free = 0;
        for (unsigned long j = 0; j < ARENA_POOL_SIZE; ++j) {
            int b = 1ULL << j;
            if (pool->mask & b) {
                ++free;
            }
        }
        ++count;
        // fprintf(fp, "===  EnvPool at %p, free slots %d, mask %0*" PRIx64 "\n", pool, free, (int) (ARENA_POOL_SIZE * 2 / sizeof(uint64_t)), pool->mask);
    }
    fprintf(fp, "===  EnvPool count: %d\n", count);
}

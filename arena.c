#include <strings.h>
#include "mem.h"
#include "arena.h"

#define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"

Arena* arena_create(void)
{
    Arena* arena = 0;
    MEM_ALLOC_TYPE(arena, 1, Arena);
    LOG(INFO, ("arena: created %p", arena));
    return arena;
}

void arena_destroy(Arena* arena)
{
    LOG(INFO, ("arena: destroying arena %p with %d pools", arena, arena->size));
    for (Pool* pool = arena->cells; pool; ) {
        Pool* tmp = pool;
        pool = pool->next;
        LOG(INFO, ("arena: destroying pool %p", tmp));
        MEM_FREE_TYPE(tmp, 1, Pool);
    }
    MEM_FREE_TYPE(arena, 1, Arena);
}

Cell* arena_get_cell(Arena* arena, int hint)
{
    (void) hint;
    Pool* pool = 0;
    int pos = 0;
    for (pool = arena->cells; pool; pool = pool->next) {
        // find first unused slot in this pool and return it
        // NOTE: from man page for fls:
        // The ffs(), ffsl() and ffsll() functions find the first (least
        // significant) bit set in value and return the index of that bit.
        // Bits are numbered starting at 1, the least significant bit.  A
        // return value of zero from any of these functions means that the
        // argument was zero.
        int lsb = ffsll(pool->mask);
        if (lsb) { // same as pool->mask == POOL_FULL
            pos = lsb - 1;
            break;
        }
    }

    if (!pool) {
        // Need to create a new pool
        MEM_ALLOC_TYPE(pool, 1, Pool);
        LOG(INFO, ("arena: created pool #%d: %p", arena->size, pool));
        pool->mask = POOL_EMPTY;
        pool->next = arena->cells;
        arena->cells = pool;
        ++arena->size;
        pos = 0;
    }

    // mark pos as used and return it
    POOL_MARK_USED(pool->mask, pos);
    return &pool->slots[pos];
}

void arena_reset_to_empty(Arena* arena)
{
    for (Pool* pool = arena->cells; pool; pool = pool->next) {
        pool->mask = POOL_EMPTY;
    }
}

void arena_mark_cell_used(Arena* arena, const Cell* cell)
{
    Pool* pool = arena_pool_for_cell(arena, cell);
    if (pool) {
        int pos = cell - pool->slots;
        POOL_MARK_USED(pool->mask, pos);
        return;
    }

    LOG(FATAL, ("ARENA: cell %p out of bound -- WTF?", cell));
}

Pool* arena_pool_for_cell(Arena* arena, const Cell* cell)
{
    for (Pool* pool = arena->cells; pool; pool = pool->next) {
        if (pool->slots <= cell && cell < (pool->slots + ARENA_POOL_SIZE)) {
            return pool;
        }

    }
    return 0;
}

void arena_dump(Arena* arena, FILE* fp)
{
    fprintf(fp, "Arena at %p with %d pools\n", arena, arena->size);
    for (Pool* pool = arena->cells; pool; pool = pool->next) {
        fprintf(fp, "  Pool at %p, mask %" PRIx64 "\n", pool, pool->mask);
    }
}

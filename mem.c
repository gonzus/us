#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"

static long mem_total_alloc = 0;
static long mem_total_free = 0;

static void mem_print_final_stats(void);
static void mem_check_and_register(void);

void* mem_alloc(const char* file, int line, int count, int size, int zero)
{
    mem_check_and_register();

    int total = count * size;
    void* mem = zero ? calloc(count, size) : malloc(total);
    fprintf(stderr, "MEM A %d %d %d %p %s %d\n", count, size, total, mem, file, line);
    mem_total_alloc += total;
    return mem;
}

void mem_free(const char* file, int line, int count, int size, void* mem)
{
    mem_check_and_register();

    if (size == 0 && mem) {
        size = strlen(mem) + 1;
    }
    int total = count * size;
    fprintf(stderr, "MEM F %d %d %d %p %s %d\n", count, size, total, mem, file, line);
    free((void*) mem);
    mem_total_free += total;
}

static void mem_check_and_register(void)
{
    static int registered = 0;
    if (registered) {
        return;
    }
    registered = 1;
    atexit(mem_print_final_stats);
}

static void mem_print_final_stats(void)
{
    long delta = mem_total_alloc - mem_total_free;
    fprintf(stderr, "MEM %s %ld %ld %ld\n",
            delta ? "BAD" : "OK", mem_total_alloc, mem_total_free, delta);
}

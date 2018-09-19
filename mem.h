#ifndef MEM_H_
#define MEM_H_

#include <string.h>

#if !defined(MEM_DEBUG)
#define MEM_DEBUG 0
#endif

#if defined(MEM_DEBUG) && MEM_DEBUG > 0

#define MEM_ALLOC_TYPE(v, c, t) \
    do { \
        v = (t*) mem_alloc(__FILE__, __LINE__, c, sizeof(t), 1); \
    } while (0)
#define MEM_ALLOC_SIZE(v, s) \
    do { \
        v = (char*) mem_alloc(__FILE__, __LINE__, 1, s, 0); \
    } while (0)
#define MEM_ALLOC_STRDUP(v, s) \
    do { \
        int l = strlen(s) + 1; \
        v = (char*) mem_alloc(__FILE__, __LINE__, 1, l, 0); \
        memcpy(v, s, l); \
    } while (0)
#define MEM_FREE_TYPE(v, c, t) \
    do { \
        mem_free(__FILE__, __LINE__, c, sizeof(t), (void*) v); \
        v = 0; \
    } while (0)
#define MEM_FREE_SIZE(v, s) \
    do { \
        mem_free(__FILE__, __LINE__, 1, s, (void*) v); \
        v = 0; \
    } while (0)

#else

#include <stdlib.h>

#define MEM_ALLOC_TYPE(v, c, t) \
    do { \
        v = (t*) calloc(c, sizeof(t)); \
    } while (0)
#define MEM_ALLOC_SIZE(v, s) \
    do { \
        v = (char*) malloc(s); \
    } while (0)
#define MEM_ALLOC_STRDUP(v, s) \
    do { \
        int l = strlen(s) + 1; \
        v = (char*) malloc(l); \
        memcpy(v, s, l); \
    } while (0)

#define MEM_FREE_TYPE(v, c, t) \
    do { \
        free((void*) v); \
        v = 0; \
    } while (0)
#define MEM_FREE_SIZE(v, s) \
    do { \
        free((void*) v); \
        v = 0; \
    } while (0)

#endif

void* mem_alloc(const char* file, int line, int count, int size, int zero);
void mem_free(const char* file, int line, int count, int size, void* mem);

#endif

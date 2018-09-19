#include <string.h>
#include "cell.h"
#include "env.h"

#if !defined(MEM_DEBUG)
#define MEM_DEBUG 0
#endif
#include "mem.h"

// #define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"
#if defined(LOG_LEVEL) && LOG_LEVEL <= LOG_LEVEL_INFO
static char dumper[10*1024];
#endif

// Default size for the environment's hash table.
// It is a prime number below 1024.
#define ENV_DEFAULT_SIZE 1021

static unsigned long hash(const char* str);

void env_destroy(Env* env)
{
    LOG(INFO, ("ENV: destroying %p, %d buckets, parent %p", env, env->size, env->parent));
    for (int j = 0; j < env->size; ++j) {
        for (Symbol* sym = env->table[j]; sym != 0; ) {
            LOG(INFO, ("ENV: %5d: [%s] => [%s]\n", j, sym->name, cell_dump(sym->value, 1, dumper)));
            Symbol* tmp = sym;
            sym = sym->next;
            MEM_FREE_SIZE(tmp->name, 0);
            MEM_FREE_TYPE(tmp, 1, Symbol);
        }
    }
    MEM_FREE_TYPE(env->table, env->size, Symbol*);
    env->table = 0;
    env->size = 0;
    env->parent = 0;
    MEM_FREE_TYPE(env, 1, Env);
}

Env* env_create(int size)
{
    Env* env = 0;
    MEM_ALLOC_TYPE(env, 1, Env);
    env->size = size <= 0 ? ENV_DEFAULT_SIZE : size;
    MEM_ALLOC_TYPE(env->table, env->size, Symbol*);
    LOG(INFO, ("ENV: created %p, %d buckets", env, env->size));
    return env;
}

void env_chain(Env* env, Env* parent)
{
    if (!parent) {
        return;
    }

    env->parent = parent;
    LOG(INFO, ("ENV: chained %p to parent %p", env, env->parent));
}

Symbol* env_lookup(Env* env, const char* name, int create)
{
    // Search for name in current env
    int h = hash(name) % env->size;
    Symbol* sym = 0;
    for (sym = env->table[h]; sym != 0; sym = sym->next) {
        if (strcmp(name, sym->name) == 0) {
            return sym;
        }
    }

    // Name not found, search for it up in the chain, but NEVER create it there
    if (env->parent) {
        sym = env_lookup(env->parent, name, 0);
        if (sym) {
            return sym;
        }
    }

    // Not found so far, maybe create it?
    if (create) {
        MEM_ALLOC_TYPE(sym, 1, Symbol);
        MEM_ALLOC_STRDUP(sym->name, name);
        sym->next = env->table[h];
        env->table[h] = sym;
        LOG(INFO, ("Created sym [%s]", name));
    }

    // Return what we got, if anything
    return sym;
}

void env_dump(Env* env, FILE* fp)
{
    fprintf(fp, "Env %p, %d buckets, parent %p\n", env, env->size, env->parent);
    for (int j = 0; j < env->size; ++j) {
        for (Symbol* sym = env->table[j]; sym != 0; sym = sym->next) {
            fprintf(fp, "%5d: [%s] => [%s]\n", j, sym->name, cell_dump(sym->value, 1, dumper));
        }
    }
}

// I've had nice results with djb2 by Dan Bernstein.
static unsigned long hash(const char* str)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

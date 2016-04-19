#include <stdlib.h>
#include <string.h>
#include "cell.h"
#include "env.h"

void env_destroy(Env* env)
{
    for (int j = 0; j < env->size; ++j) {
        Symbol* t = 0;
        for (Symbol* s = env->table[j]; s != 0; ) {
            t = s;
            s = s->next;
            free(t);
        }
    }
    free(env->table);
    env->table = 0;
    env->size = 0;
    env->parent = 0;
    free(env);
}

Env* env_create(int size, Env* parent)
{
    Env* env = (Env*) malloc(sizeof(Env));
    env->parent = parent;
    env->size = size <= 0 ? ENV_DEFAULT_SIZE : size;
    env->table = calloc(env->size, sizeof(Symbol));
    return env;
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

Symbol* env_lookup(Env* env, const char* name, int create)
{
    // Search for name in current env
    int h = hash(name) % env->size;
    Symbol* s = 0;
    for (s = env->table[h]; s != 0; s = s->next) {
        if (strcmp(name, s->name) == 0) {
            return s;
        }
    }

    // Name not found, search for it up in the chain, but NEVER create it there
    if (env->parent) {
        s = env_lookup(env->parent, name, 0);
        if (s) {
            return s;
        }
    }

    // Not found so far, maybe create it?
    if (create) {
        s = (Symbol*) malloc(sizeof(Symbol));
        s->name = strdup(name);
        s->value = 0;
        s->next = env->table[h];
        env->table[h] = s;
    }

    // Return what we got, if anything
    return s;
}

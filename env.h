/*
 */
#ifndef ENV_H_
#define ENV_H_

#define ENV_DEFAULT_SIZE 1021

typedef struct Symbol {
    const char* name;
    struct Cell* value;
    struct Symbol* next;
} Symbol;

typedef struct Env {
    Symbol** table;
    int size;
    struct Env* parent;
} Env;

Env* env_create(int size, Env* parent);
void env_destroy(Env* env);

Symbol* env_lookup(Env* env, const char* name, int create);

#endif

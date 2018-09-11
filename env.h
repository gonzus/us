#ifndef ENV_H_
#define ENV_H_

// An environment is a hash table that stores associations of name => value.
// It also can have a parent environment.
// When names hash to the same bucket, use a singly linked list.

#include "cell.h"

// An entry in the hash table
typedef struct Symbol {
    const char* name;
    Cell* value;
    struct Symbol* next;
} Symbol;

// The environment itself:
typedef struct Env {
    Symbol** table;     // Hash table buckets
    int size;           // Size for hash table
    struct Env* parent; // Pointer to (possible) parent environment
} Env;

// Destroy an environment
void env_destroy(Env* env);

// Create a new environment, given its size and an optional parent
Env* env_create(int size, Env* parent);

// Seach for a given name in the environment.
// If not found, search up the chain of parents.
// If not found anywhere and create is true, create new entry.
// Return the Symbol (possibly empty) associated with this name.
Symbol* env_lookup(Env* env, const char* name, int create);

#endif

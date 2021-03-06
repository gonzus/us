#ifndef ENV_H_
#define ENV_H_

#include <stdio.h> // need this for FILE*

// An environment is a hash table that stores associations of name => value.
// It also can have a parent environment.
// When names hash to the same bucket, use a singly linked list.

// Define our structures
struct US;
struct Cell;
struct Env;

// An entry in the hash table
typedef struct Symbol {
    char* name;
    struct Cell* value;
    struct Symbol* next;
} Symbol;

// The environment itself:
typedef struct Env {
    Symbol** table;     // hash table buckets
    int size;           // size for hash table
    struct Env* parent; // pointer to (possible) parent environment
} Env;

// Destroy an environment
void env_destroy(Env* env);

// Create a new environment, given its (optional) size
Env* env_create(int size);

// Chain an environment with a parent
void env_chain(Env* env, Env* parent);

// Search for a given name in the environment.
// If not found, search up the chain of parents.
// If not found anywhere and create is non-zero, create a new entry.
// Return the Symbol (valid but possibly empty) associated with this name.
Symbol* env_lookup(Env* env, const char* name, int create);

// Dump environmnet
void env_dump(Env* env, FILE* fp);

#endif

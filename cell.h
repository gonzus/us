#ifndef CELL_H_
#define CELL_H_

// need this for FILE*
#include <stdio.h>

// A Cell stores any possible value (integer, conses, others)
// using a union.
//
// In particular, it supports values of type cons.
//
// WARNING: we are not doing any kind of memory management.
// This thing leaks like a boat made of hay.
// Need to decide wether we will use ref counting or GC.

// Types of cell
#define CELL_NONE   0  // Needed? Special value nil uses it
#define CELL_INT    1  // Integers => long
#define CELL_REAL   2  // Reals => double
#define CELL_STRING 3  // Strings
#define CELL_SYMBOL 4  // Symbols
#define CELL_CONS   5  // Cons cells
#define CELL_PROC   6  // Procedures (interpreted code)
#define CELL_NATIVE 7  // Native functions (compiled code)
#define CELL_LAST   8

// Printable forms of these special values
#define CELL_STR_NIL    "()"
#define CELL_STR_BOOL_T "#t"
#define CELL_STR_BOOL_F "#f"

// A cons cell; guess what these members are...
typedef struct Cons {
    const struct Cell* car;
    const struct Cell* cdr;
} Cons;

// Function prototype for native implementation of procs
typedef const struct Cell* (NativeFunc)(const struct Cell* args);

// A procedure
typedef struct Procedure {
    const struct Cell* params;
    const struct Cell* body;
    struct Env* env;
} Procedure;

// A native cell
typedef struct Native {
    const char* label;
    NativeFunc* func;
} Native;

// Finally, definition of a cell
typedef struct Cell {
    unsigned char tag;
    union {
        long ival;
        double rval;
        char* sval;
        Cons cons;
        Procedure pval;
        Native nval;
    };
} Cell;

// Let's just have a single value for nil, #t and #f
extern const Cell* nil;
extern const Cell* bool_t;
extern const Cell* bool_f;

// Destroy a cell
void cell_destroy(Cell* cell);

// Create a cell with an integer value
Cell* cell_create_int(long value);
Cell* cell_create_int_from_string(const char* value, int len);

// Create a cell with a real value
Cell* cell_create_real(double value);
Cell* cell_create_real_from_string(const char* value, int len);

// Create a cell with a string value
Cell* cell_create_string(const char* value, int len);

// Create a cell with a symbol value
Cell* cell_create_symbol(const char* value, int len);

// Create a cell with a procedure
Cell* cell_create_procedure(const Cell* params, const Cell* body, struct Env* env);

// Create a cell with a native function
Cell* cell_create_native(const char* label, NativeFunc* func);

// Implementation of cons
Cell* cell_cons(const Cell* car, const Cell* cdr);

// Implementations of car and cdr
const Cell* cell_car(const Cell* cell);
const Cell* cell_cdr(const Cell* cell);

// Print contents of cell to given stream, optionally adding a \n
void cell_print(const Cell* cell, FILE* fp, int eol);

const char* cell_dump(const Cell* cell, int debug, char* buf);

#endif

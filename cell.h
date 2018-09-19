#ifndef CELL_H_
#define CELL_H_

#include <stdio.h> // need this for FILE*

// A Cell stores any possible value (integer, conses, others) using a union.

// Types of cell
#define CELL_NONE   0  // Special value, nil uses it
#define CELL_INT    1  // Integers (stored as long)
#define CELL_REAL   2  // Reals (stored as double)
#define CELL_STRING 3  // Strings
#define CELL_SYMBOL 4  // Symbols
#define CELL_CONS   5  // Cons cells (car and cdr)
#define CELL_PROC   6  // Procedures (interpreted code)
#define CELL_NATIVE 7  // Native functions (compiled code)
#define CELL_LAST   8

// Printable forms of these special values
#define CELL_STR_NIL    "()"
#define CELL_STR_BOOL_T "#t"
#define CELL_STR_BOOL_F "#f"

#define LIST_RESET(e) \
    do { \
        (e).frst = 0; (e).last = 0; \
    } while (0)

#define LIST_APPEND(e, c) \
    do { \
        Cell* x = c; \
        if (!((e)->frst)) (e)->frst = x; \
        if ((e)->last) { \
            (e)->last->cons.cdr = x; \
        } \
        (e)->last = x; \
    } while (0)

// Define our structures
struct US;
struct Cell;
struct Env;

// Function prototype for native implementation of procs
typedef struct Cell* (NativeFunc)(struct US* us, struct Cell* args);

// A cons cell; guess what these members are...
typedef struct Cons {
    struct Cell* car;
    struct Cell* cdr;
} Cons;

// A procedure
typedef struct Procedure {
    struct Cell* params;
    struct Cell* body;
    struct Env* env;
} Procedure;

// A native cell
typedef struct Native {
    const char* label;
    NativeFunc* func;
} Native;

// Finally, definition of a cell
typedef struct Cell {
    unsigned char tag;  // type of cell
    union {
        long ival;      // an integer value
        double rval;    // a real value
        char* sval;     // a string value (string or symbol)
        Cons cons;      // a cons cell with car and cdr
        Procedure pval; // an interpreted (scheme) function
        Native nval;    // a native (C) function
    };
} Cell;

// Let's just have a single global value for nil, #t and #f
extern Cell* nil;
extern Cell* bool_t;
extern Cell* bool_f;

// Destroy a cell
void cell_destroy(struct US* us, Cell* cell);

// Create a cell with an integer value
Cell* cell_create_int(struct US* us, long value);
Cell* cell_create_int_from_string(struct US* us, const char* value, int len);

// Create a cell with a real value
Cell* cell_create_real(struct US* us, double value);
Cell* cell_create_real_from_string(struct US* us, const char* value, int len);

// Create a cell with a string value
Cell* cell_create_string(struct US* us, const char* value, int len);

// Create a cell with a symbol value
Cell* cell_create_symbol(struct US* us, const char* value, int len);

// Create a cell with a procedure
Cell* cell_create_procedure(struct US* us, Cell* params, Cell* body, struct Env* env);

// Create a cell with a native function
Cell* cell_create_native(struct US* us, const char* label, NativeFunc* func);

// Implementation of cons
Cell* cell_cons(struct US* us, Cell* car, Cell* cdr);

// Implementations of car and cdr
Cell* cell_car(Cell* cell);
Cell* cell_cdr(Cell* cell);

// Print contents of cell to given stream, optionally adding a \n
void cell_print(const Cell* cell, FILE* fp, int eol);

const char* cell_dump(const Cell* cell, int debug, char* buf);

#endif

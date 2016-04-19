#ifndef CELL_H_
#define CELL_H_

// A struct Cell stores any possible value (integer, conses, others)
// using a union.
//
// In particular, it supports values of type cons.
//
// WARNING: we are not doing any kind of memory management.
// This thing leaks like a boat made of hay.
// Need to decide wether we will use ref counting or GC.

#include <stdio.h>

#define CELL_NONE   0  // Needed?
#define CELL_INT    1  // Integers => long
#define CELL_REAL   2  // Reals => double
#define CELL_STRING 3  // Strings
#define CELL_SYMBOL 4  // Symbols
#define CELL_CONS   5  // Cons cells
#define CELL_FUNC   6  // Functions -- not used yet

// Guess what these members are...
typedef struct Cons {
    struct Cell* car;
    struct Cell* cdr;
} Cons;

// A tagged anonymous union goes in here:
typedef struct Cell {
    unsigned char tag;
    union {
        long ival;
        double rval;
        char* sval;
        Cons cons;
    };
} Cell;

// Let's just have a single nil value
extern Cell* nil;

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

// Implementation of cons
Cell* cell_cons(Cell* car, Cell* cdr);

// Implementations of car and cdr
Cell* cell_car(const Cell* cell);
Cell* cell_cdr(const Cell* cell);

// Print contents of cell to given stream, optionally adding a \n
void cell_print(const Cell* cell, FILE* fp, int eol);

#endif

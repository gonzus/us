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
#define CELL_INT    1  // Integers => int
#define CELL_CONS   2  // Cons cells
#define CELL_FUNC   3  // Functions -- not used yet

// Guess what these members are...
typedef struct Cons {
    struct Cell* car;
    struct Cell* cdr;
} Cons;

// A tagged anonymous union goes in here:
typedef struct Cell {
    unsigned char tag;
    union {
        int ival;
        Cons cons;
    };
} Cell;

// Let's just have a single nil value
extern Cell* nil;

// Destroy a cell
void cell_destroy(Cell* cell);

// Create a cell with an int value
Cell* cell_create_int(int value);

// Implementation of cons
Cell* cell_cons(Cell* car, Cell* cdr);

// Implementations of car and cdr
Cell* cell_car(const Cell* cell);
Cell* cell_cdr(const Cell* cell);

// Print contents of cell to given stream, optionally adding a \n
void cell_print(const Cell* cell, FILE* fp, int eol);

#endif

/*
 */
#ifndef CELL_H_
#define CELL_H_

#include <stdio.h>

#define CELL_NONE   0
#define CELL_INT    1
#define CELL_CONS   2
#define CELL_FUNC   3

typedef struct Cons {
    struct Cell* car;
    struct Cell* cdr;
} Cons;

typedef struct Cell {
    unsigned char tag;
    union {
        int ival;
        Cons cons;
    };
} Cell;

extern Cell* nil;

void cell_destroy(Cell* cell);

Cell* cell_create_int(int value);
Cell* cell_cons(Cell* car, Cell* cdr);

Cell* cell_car(const Cell* cell);
Cell* cell_cdr(const Cell* cell);

void cell_print(const Cell* cell, FILE* fp, int eol);

#endif

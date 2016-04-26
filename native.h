#ifndef NATIVE_H_
#define NATIVE_H_

// Native implementations of several procedures

Cell* func_add(Cell* args);
Cell* func_sub(Cell* args);
Cell* func_mul(Cell* args);
Cell* func_div(Cell* args);

// TODO: find out if this should be
// eq, eqv, equals, or something else...
Cell* func_eq(Cell* args);

Cell* func_gt(Cell* args);
Cell* func_lt(Cell* args);

Cell* func_cons(Cell* args);
Cell* func_car(Cell* args);
Cell* func_cdr(Cell* args);

Cell* func_begin(Cell* args);

#endif

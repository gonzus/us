#ifndef NATIVE_H_
#define NATIVE_H_

// Native implementations of several procedures

const struct Cell* func_add(const struct Cell* args);
const struct Cell* func_sub(const struct Cell* args);
const struct Cell* func_mul(const struct Cell* args);
const struct Cell* func_div(const struct Cell* args);

// TODO: find out if this should be
// eq, eqv, equals, or something else...
const struct Cell* func_eq(const struct Cell* args);

const struct Cell* func_gt(const struct Cell* args);
const struct Cell* func_lt(const struct Cell* args);

const struct Cell* func_cons(const struct Cell* args);
const struct Cell* func_car(const struct Cell* args);
const struct Cell* func_cdr(const struct Cell* args);

const struct Cell* func_begin(const struct Cell* args);

#endif

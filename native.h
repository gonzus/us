#ifndef NATIVE_H_
#define NATIVE_H_

struct US;

// Native implementations of several procedures

const struct Cell* func_add(struct US* us, const struct Cell* args);
const struct Cell* func_sub(struct US* us, const struct Cell* args);
const struct Cell* func_mul(struct US* us, const struct Cell* args);
const struct Cell* func_div(struct US* us, const struct Cell* args);

// TODO: find out if this should be
// eq, eqv, equals, or something else...
const struct Cell* func_eq(struct US* us, const struct Cell* args);

const struct Cell* func_gt(struct US* us, const struct Cell* args);
const struct Cell* func_lt(struct US* us, const struct Cell* args);

const struct Cell* func_cons(struct US* us, const struct Cell* args);
const struct Cell* func_car(struct US* us, const struct Cell* args);
const struct Cell* func_cdr(struct US* us, const struct Cell* args);

const struct Cell* func_begin(struct US* us, const struct Cell* args);

#endif

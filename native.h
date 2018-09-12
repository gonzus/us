#ifndef NATIVE_H_
#define NATIVE_H_

// Native implementations of several procedures

struct Cell* func_add(struct Cell* args);
struct Cell* func_sub(struct Cell* args);
struct Cell* func_mul(struct Cell* args);
struct Cell* func_div(struct Cell* args);

// TODO: find out if this should be
// eq, eqv, equals, or something else...
struct Cell* func_eq(struct Cell* args);

struct Cell* func_gt(struct Cell* args);
struct Cell* func_lt(struct Cell* args);

struct Cell* func_cons(struct Cell* args);
struct Cell* func_car(struct Cell* args);
struct Cell* func_cdr(struct Cell* args);

struct Cell* func_begin(struct Cell* args);

#endif

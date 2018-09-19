#ifndef NATIVE_H_
#define NATIVE_H_

// Define our structures
struct US;
struct Cell;

// Native implementations of several procedures

struct Cell* func_add(struct US* us, struct Cell* args);
struct Cell* func_sub(struct US* us, struct Cell* args);
struct Cell* func_mul(struct US* us, struct Cell* args);
struct Cell* func_div(struct US* us, struct Cell* args);

// TODO: find out if this should be
// eq, eqv, equals, or something else...
struct Cell* func_eq(struct US* us, struct Cell* args);

struct Cell* func_gt(struct US* us, struct Cell* args);
struct Cell* func_lt(struct US* us, struct Cell* args);

struct Cell* func_cons(struct US* us, struct Cell* args);
struct Cell* func_car(struct US* us, struct Cell* args);
struct Cell* func_cdr(struct US* us, struct Cell* args);

struct Cell* func_begin(struct US* us, struct Cell* args);

#endif

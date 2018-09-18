#ifndef EVAL_H_
#define EVAL_H_

struct US;

// Eval the expresion contained in a cell, given a
// specific environment.
// Warning: magic happens here!
const struct Cell* cell_eval(struct US* us, const struct Cell* cell, struct Env* env);

#endif

#ifndef EVAL_H_
#define EVAL_H_

// Eval the expresion contained in a cell, given a
// specific environment.
// Warning: magic happens here!
struct Cell* cell_eval(struct Cell* cell, struct Env* env);

#endif

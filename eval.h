#ifndef EVAL_H_
#define EVAL_H_

// Define our structures
struct US;
struct Cell;
struct Env;

// Eval the expresion in a cell, given a specific environment. Magic!
Cell* cell_eval(struct US* us, struct Cell* cell, struct Env* env);

#endif

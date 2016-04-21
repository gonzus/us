#ifndef EVAL_H_
#define EVAL_H_

#include "cell.h"
#include "env.h"

// Eval the expresion contained in a cell, given a
// specific environment.
// Warning: magic happens here!
Cell* cell_eval(Cell* cell, Env* env);

#endif

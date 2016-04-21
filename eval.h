#ifndef EVAL_H_
#define EVAL_H_

#include "cell.h"
#include "env.h"

// Magic happens here!
Cell* cell_eval(Cell* cell, Env* env);

#endif

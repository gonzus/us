#ifndef EVAL_H_
#define EVAL_H_

#include "cell.h"
#include "env.h"

// Magic happens here!
const Cell* cell_eval(const Cell* cell, Env* env);

#endif

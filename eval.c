#include <stdio.h>
#include <string.h>
#include "eval.h"

#define EVAL_QUOTE  "quote"
#define EVAL_IF     "if"
#define EVAL_DEFINE "define"

const Cell* cell_eval(const Cell* cell, Env* env)
{
    printf("Called eval for ");
    cell_print(cell, stdout, 1);

    Symbol* symbol = 0;
    const Cell* ret = nil;
    switch (cell->tag) {
        case CELL_INT:
        case CELL_REAL:
        case CELL_STRING:
            ret = cell;
            break;

        case CELL_SYMBOL:
            symbol = env_lookup(env, cell->sval, 0);
            ret = symbol->value;
            break;

        case CELL_CONS:
            cell = cell->cons.car;
            switch (cell->tag) {
                case CELL_SYMBOL:
                    if (strcmp(cell->sval, EVAL_QUOTE) == 0) {
                        printf("QUOTE not implemented\n");
                    }
                    else if (strcmp(cell->sval, EVAL_IF) == 0) {
                        printf("IF not implemented\n");
                    }
                    else if (strcmp(cell->sval, EVAL_DEFINE) == 0) {
                        printf("DEFINE not implemented\n");
                    }
                    else {
                        symbol = env_lookup(env, cell->sval, 0);
                        cell = cell_eval(symbol->value, env);
                        // TODO: eval args, call cell which should be a func
                    }
                    break;
            }
            break;

        case CELL_NATIVE:
            // TODO: pass args
            cell = cell->nval.func(0);
            break;
    }

    printf("Finished eval, returning ");
    cell_print(ret, stdout, 1);

    return ret;
}


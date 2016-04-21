#include <stdio.h>
#include <string.h>
#include "eval.h"

#define EVAL_QUOTE  "quote"
#define EVAL_IF     "if"
#define EVAL_DEFINE "define"

static Cell* cell_apply(Cell* cell, Env* env);

Cell* cell_eval(Cell* cell, Env* env)
{
    Symbol* symbol = 0;
    Cell* ret = nil;
    Cell* car = nil;
    switch (cell->tag) {
        case CELL_INT:
        case CELL_REAL:
        case CELL_STRING:
        case CELL_NATIVE:
            ret = cell;
            break;

        case CELL_SYMBOL:
            symbol = env_lookup(env, cell->sval, 0);
            if (symbol) {
                ret = symbol->value;
            }
            break;

        case CELL_CONS:
            car = cell->cons.car;
            switch (car->tag) {
                case CELL_SYMBOL:
                    if (strcmp(car->sval, EVAL_QUOTE) == 0) {
                        printf("QUOTE not implemented\n");
                    }
                    else if (strcmp(car->sval, EVAL_IF) == 0) {
                        printf("IF not implemented\n");
                    }
                    else if (strcmp(car->sval, EVAL_DEFINE) == 0) {
                        printf("DEFINE not implemented\n");
                    }
                    else {
                        ret = cell_apply(cell, env);
                    }
                    break;
            }
            break;
    }

    return ret;
}

static Cell* cell_apply(Cell* cell, Env* env)
{
    Cell* ret = nil;
    if (!cell || cell->tag != CELL_CONS) {
        return ret;
    }
    Cell* proc = cell_eval(cell->cons.car, env);
    if (!proc) {
        return ret;
    }
    Cell* args = 0;
    Cell* last = 0;
    for (Cell* c = cell->cons.cdr; c && c!= nil; c = c->cons.cdr) {
        Cell* arg = cell_eval(c->cons.car, env);
        if (!arg) {
            // TODO: error
            printf("You stupid motherfucker\n");
            continue;
        }
        Cell* cons = cell_cons(arg, nil);
        if (!args) {
            args = cons;
        }
        if (!last) {
            last = cons;
        }
        else {
            last->cons.cdr = cons;
            last = cons;
        }
    }
    switch (proc->tag) {
        case CELL_NATIVE:
            ret = proc->nval.func(args);
            break;

        default:
            // TODO: error
            printf("You stupid motherfucker\n");
            break;
    }
    return ret;
}


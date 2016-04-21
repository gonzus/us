#include <stdio.h>
#include <string.h>
#include "eval.h"

#define EVAL_QUOTE  "quote"
#define EVAL_IF     "if"
#define EVAL_DEFINE "define"
#define EVAL_SET    "set!"
#define EVAL_LAMBDA "lambda"

static Cell* cell_apply(Cell* cell, Env* env);
static Cell* cell_set_value(Cell* cell, Env* env, int create);

Cell* cell_eval(Cell* cell, Env* env)
{
    Symbol* symbol = 0;
    Cell* ret = nil;
    Cell* car = nil;
    switch (cell->tag) {
        // literals
        case CELL_INT:
        case CELL_REAL:
        case CELL_STRING:
        case CELL_NATIVE:
            ret = cell;
            break;

        // variable reference
        case CELL_SYMBOL:
            symbol = env_lookup(env, cell->sval, 0);
            if (symbol) {
                ret = symbol->value;
            }
            break;

        // a list; action will depend on its car
        case CELL_CONS:
            car = cell->cons.car;
            switch (car->tag) {
                case CELL_SYMBOL:
                    if (strcmp(car->sval, EVAL_QUOTE) == 0) {
                        // a quote special form
                        if (cell->cons.cdr) {
                            ret = cell->cons.cdr->cons.car;
                        }
                    }
                    else if (strcmp(car->sval, EVAL_DEFINE) == 0) {
                        // a define special form
                        ret = cell_set_value(cell, env, 1);
                    }
                    else if (strcmp(car->sval, EVAL_SET) == 0) {
                        // a set! special form
                        ret = cell_set_value(cell, env, 0);
                    }
                    else if (strcmp(car->sval, EVAL_IF) == 0     ||
                             strcmp(car->sval, EVAL_LAMBDA) == 0 ) {
                        printf("%s not implemented\n", car->sval);
                    }
                    else {
                        // a function invocation
                        ret = cell_apply(cell, env);
                    }
                    break;
            }
            break;
    }

    return ret;
}

// Apply a function (car) to all its arguments (cdr)
static Cell* cell_apply(Cell* cell, Env* env)
{
    Cell* ret = nil;
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
        if (last) {
            last->cons.cdr = cons;
        }
        last = cons;
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

// Set a value in the given environment;
// optionally create it if it doesn't exist.
static Cell* cell_set_value(Cell* cell, Env* env, int create)
{
    Cell* ret = nil;
    do {
        Cell* args[3];
        int pos = 0;
        for (Cell* c = cell; c && c != nil && pos < 3; c = c->cons.cdr) {
            args[pos++] = c->cons.car;
        }
        if (!args[1] || !args[2]) {
            break;
        }

        Symbol* symbol = env_lookup(env, args[1]->sval, create);
        if (!symbol) {
            break;
        }

        ret = cell_eval(args[2], env);
        symbol->value = ret;
    } while (0);

    return ret;
}


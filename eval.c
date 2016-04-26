#include <stdio.h>
#include <string.h>
#include "eval.h"

#define EVAL_QUOTE  "quote"
#define EVAL_IF     "if"
#define EVAL_DEFINE "define"
#define EVAL_SET    "set!"
#define EVAL_LAMBDA "lambda"

static Cell* cell_apply(Cell* cell, Env* env);
static Cell* cell_apply_proc(Cell* cell, Env* env, Cell* proc);
static Cell* cell_apply_native(Cell* cell, Env* env, Cell* proc);
static Cell* cell_set_value(Cell* cell, Env* env, int create);
static Cell* cell_if(Cell* cell, Env* env);
static Cell* cell_lambda(Cell* cell, Env* env);

Cell* cell_eval(Cell* cell, Env* env)
{
    if (cell->tag == CELL_SYMBOL) {
        Cell* ret = nil;
        Symbol* symbol = env_lookup(env, cell->sval, 0);
        if (symbol) {
            ret = symbol->value;
            printf("EVAL: looked up symbol [%s] in env %p => %p (tag %d)\n", cell->sval, env, ret, ret->tag);
        }
        return ret;
    }

    if (cell->tag != CELL_CONS) {
        return cell;
    }

    Cell* car = cell->cons.car;
    printf("EVAL: evaluating a cons cell, car is %p (tag %d)\n", car, car->tag);
    int done = 0;
    Cell* ret = nil;
    if (car->tag == CELL_SYMBOL) {
        if (strcmp(car->sval, EVAL_QUOTE) == 0) {
            // a quote special form
            done = 1;
            if (cell->cons.cdr) {
                ret = cell->cons.cdr->cons.car;
            }
        }
        else if (strcmp(car->sval, EVAL_DEFINE) == 0) {
            // a define special form
            done = 1;
            ret = cell_set_value(cell, env, 1);
        }
        else if (strcmp(car->sval, EVAL_SET) == 0) {
            // a set! special form
            done = 1;
            ret = cell_set_value(cell, env, 0);
        }
        else if (strcmp(car->sval, EVAL_IF) == 0) {
            // an if special form
            done = 1;
            ret = cell_if(cell, env);
        }
        else if (strcmp(car->sval, EVAL_LAMBDA) == 0 ) {
            // a lambda special form
            done = 1;
            ret = cell_lambda(cell, env);
        }
    }

    if (done) {
        return ret;
    }

    // a function invocation
    ret = cell_apply(cell, env);
    return ret;
}

// Apply a function (car) to all its arguments (cdr)
static Cell* cell_apply(Cell* cell, Env* env)
{
    Cell* proc = cell_eval(cell->cons.car, env);
    if (!proc) {
        return nil;
    }
    switch (proc->tag) {
        case CELL_PROC:
            return cell_apply_proc(cell, env, proc);

        case CELL_NATIVE:
            return cell_apply_native(cell, env, proc);
    }
    return nil;
}

static Cell* cell_apply_proc(Cell* cell, Env* env, Cell* proc)
{
    // We create a new env with no parent, so that we will bind
    // all args in fresh slots for the params (see below)
    Env* proc_env = env_create(0, 0);
    Cell* ret = nil;
    Cell* p;
    Cell* a;
    printf("EVAL: proc on %p\n", cell);
    int pos = 0;
    for (p = proc->pval.params, a = cell->cons.cdr;
         p && p != nil && a && a != nil;
         p = p->cons.cdr, a = a->cons.cdr, ++pos) {
        Cell* par = p->cons.car;
        Cell* arg = cell_eval(a->cons.car, env);
        if (!arg) {
            printf("Could not evaluate arg #%d [%s]\n", pos, par->sval);
            return nil;
        }
        Symbol* sym = env_lookup(proc_env, par->sval, 1);
        if (!sym) {
            printf("Could not create binding for arg #%d [%s]\n", pos, par->sval);
            return nil;
        }
        sym->value = arg;
        printf("Proc, setting arg #%d [%s] to %p (tag %d)\n", pos, par->sval, arg, arg->tag);
    }
    // and now we set this env's parent
    proc_env->parent = proc->pval.env;
    ret = cell_eval(proc->pval.body, proc_env);

    // TODO: check if below comment is still valid
    // We cannot destroy the proc_env variable, because it may have been
    // "captured" and will be returned to the caller; this  happens when
    // returning a lambda as the result of calling a procedure
    return ret;
}

static Cell* cell_apply_native(Cell* cell, Env* env, Cell* proc)
{
    Cell* ret = nil;
    Cell* args = 0;
    Cell* last = 0;
    printf("EVAL: native [%s] on %p\n", proc->nval.label, cell);
    int pos = 0;
    for (Cell* c = cell->cons.cdr; c && c != nil; c = c->cons.cdr, ++pos) {
        Cell* arg = cell_eval(c->cons.car, env);
        if (!arg) {
            printf("Could not evaluate arg #%d\n", pos);
            return ret;
        }
        Cell* cons = cell_cons(arg, nil);
        if (!args) {
            args = cons;
        }
        if (last) {
            last->cons.cdr = cons;
        }
        last = cons;
        printf("Native, arg #%d is %p (tag %d), wrapped in %p\n", pos, arg, arg->tag, cons);
    }
    printf("Native, calling with args %p (tag %d)\n", args, args->tag);
    ret = proc->nval.func(args);
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
        if (pos != 3 || !args[1] || !args[2]) {
            break;
        }

        printf("EVAL: set value for [%s] (%d)\n", args[1]->sval, create);
        Symbol* symbol = env_lookup(env, args[1]->sval, create);
        if (!symbol) {
            break;
        }

        ret = cell_eval(args[2], env);
        symbol->value = ret;
        printf("Setting value [%s] to %p (tag %d)\n", args[1]->sval, ret, ret->tag);
    } while (0);

    return ret;
}

// Execute an if special form
static Cell* cell_if(Cell* cell, Env* env)
{
    Cell* ret = nil;
    do {
        Cell* args[4];
        int pos = 0;
        for (Cell* c = cell; c && c != nil && pos < 4; c = c->cons.cdr) {
            args[pos++] = c->cons.car;
        }
        if (pos != 4 || !args[1] || !args[2] || !args[3]) {
            break;
        }

        Cell* test = cell_eval(args[1], env);
        printf("EVAL: if => %p %p %p\n", test, bool_t, bool_f);
        ret = cell_eval(test == bool_f ? args[3] : args[2], env);
    } while (0);

    return ret;
}

// Execute a lambda special form
static Cell* cell_lambda(Cell* cell, Env* env)
{
    Cell* ret = nil;
    do {
        Cell* args[3];
        int pos = 0;
        for (Cell* c = cell; c && c != nil && pos < 3; c = c->cons.cdr) {
            args[pos++] = c->cons.car;
        }
        if (pos != 3 || !args[1] || !args[2]) {
            break;
        }

        printf("EVAL: lambda\n");
        ret = cell_create_procedure(args[1], args[2], env);
    } while (0);

    return ret;
}

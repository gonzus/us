#include <stdio.h>
#include <string.h>
#include "eval.h"

#define EVAL_QUOTE  "quote"
#define EVAL_IF     "if"
#define EVAL_DEFINE "define"
#define EVAL_SET    "set!"
#define EVAL_LAMBDA "lambda"

static Cell* cell_quote(Cell* cell, Env* env);
static Cell* cell_apply(Cell* cell, Env* env);
static Cell* cell_apply_proc(Cell* cell, Env* env, Cell* proc);
static Cell* cell_apply_native(Cell* cell, Env* env, Cell* proc);
static Cell* cell_set_value(Cell* cell, Env* env, int create);
static Cell* cell_if(Cell* cell, Env* env);
static Cell* cell_lambda(Cell* cell, Env* env);
static int gather_args(Cell* cell, int wanted, Cell* args[]);

Cell* cell_eval(Cell* cell, Env* env)
{
    if (cell->tag == CELL_SYMBOL) {
        Symbol* symbol = env_lookup(env, cell->sval, 0);
        if (symbol) {
            Cell* ret = symbol->value;
            printf("EVAL: looked up symbol [%s] in env %p => ", cell->sval, env);
            cell_dump(ret, stdout, 1);
            return ret;
        }
        return nil;
    }

    if (cell->tag != CELL_CONS) {
        return cell;
    }

    Cell* car = cell->cons.car;
    printf("EVAL: evaluating a cons cell, car is ");
    cell_dump(car, stdout, 1);
    if (car->tag == CELL_SYMBOL) {
        if (strcmp(car->sval, EVAL_QUOTE) == 0) {
            // a quote special form
            return cell_quote(cell, env);
        }
        if (strcmp(car->sval, EVAL_DEFINE) == 0) {
            // a define special form
            return cell_set_value(cell, env, 1);
        }
        if (strcmp(car->sval, EVAL_SET) == 0) {
            // a set! special form
            return cell_set_value(cell, env, 0);
        }
        if (strcmp(car->sval, EVAL_IF) == 0) {
            // an if special form
            return cell_if(cell, env);
        }
        if (strcmp(car->sval, EVAL_LAMBDA) == 0 ) {
            // a lambda special form
            return cell_lambda(cell, env);
        }
    }

    // a function invocation
    return cell_apply(cell, env);
}

// Execute a quote special form
static Cell* cell_quote(Cell* cell, Env* env)
{
    Cell* cdr = cell->cons.cdr;
    Cell* ret = nil;
    if (cdr) {
        ret = cdr->cons.car;
    }
    printf("quote: ");
    cell_dump(ret, stdout, 1);
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
    Cell* p = 0;
    Cell* a = 0;
    printf("EVAL: proc on ");
    cell_dump(cell, stdout, 1);
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
        printf("Proc, setting arg #%d [%s] to ", pos, par->sval);
        cell_dump(arg, stdout, 1);
    }
    // and only *now* we set this env's parent
    env_chain(proc_env, proc->pval.env);

    // and finally eval the proc body in this newly created env
    ret = cell_eval(proc->pval.body, proc_env);

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
    printf("EVAL: native [%s] on ", proc->nval.label);
    cell_dump(cell, stdout, 1);
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
        printf("Native, arg #%d is ", pos);
        cell_dump(arg, stdout, 1);
    }
    printf("Native, calling with args ");
    cell_dump(args, stdout, 1);
    ret = proc->nval.func(args);
    return ret;
}

// Set a value in the given environment;
// optionally create it if it doesn't exist.
static Cell* cell_set_value(Cell* cell, Env* env, int create)
{
    Cell* args[3];
    if (!gather_args(cell, 3, args)) {
        return nil;
    }

    printf("EVAL: set value for [%s] (create: %d) to ", args[1]->sval, create);
    cell_dump(args[2], stdout, 1);
    Symbol* symbol = env_lookup(env, args[1]->sval, create);
    if (!symbol) {
        printf("EVAL: symbol [%s] not found\n", args[1]->sval);
        return nil;
    }

    Cell* ret = cell_eval(args[2], env);
    symbol->value = ret;
    printf("Setting value [%s] to ", args[1]->sval);
    cell_dump(ret, stdout, 1);
    return ret;
}

// Execute an if special form
static Cell* cell_if(Cell* cell, Env* env)
{
    Cell* args[4];
    if (!gather_args(cell, 4, args)) {
        return nil;
    }

    printf("EVAL: if Q ");
    cell_dump(args[1], stdout, 1);
    printf("EVAL: if ? ");
    cell_dump(args[2], stdout, 1);
    printf("EVAL: if : ");
    cell_dump(args[3], stdout, 1);

    Cell* tst = cell_eval(args[1], env);
    Cell* ans = cell_eval(tst == bool_t ? args[2] : args[3], env);
    printf("EVAL: if ");
    cell_dump(tst, stdout, 0);
    printf(" => ");
    cell_dump(ans, stdout, 1);
    return ans;
}

// Execute a lambda special form
static Cell* cell_lambda(Cell* cell, Env* env)
{
    Cell* args[3];
    if (!gather_args(cell, 3, args)) {
        return nil;
    }

    printf("EVAL: lambda args ");
    cell_dump(args[1], stdout, 1);
    printf("EVAL: lambda body ");
    cell_dump(args[2], stdout, 1);
    return cell_create_procedure(args[1], args[2], env);
}

static int gather_args(Cell* cell, int wanted, Cell* args[])
{
    int pos = 0;
    for (Cell* c = cell; c && c != nil && pos < wanted; c = c->cons.cdr) {
        args[pos++] = c->cons.car;
    }
    if (pos != wanted) {
        return 0;
    }
    for (int j = 1; j < wanted; ++j) {
        if (!args[j]) {
            return 0;
        }
    }

    return 1;
}

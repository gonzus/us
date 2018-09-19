#include <string.h>
#include "us.h"
#include "arena.h"
#include "cell.h"
#include "env.h"
#include "parser.h"
#include "eval.h"

// #define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"
#if defined(LOG_LEVEL) && LOG_LEVEL <= LOG_LEVEL_DEBUG
static char dumper[10*1024];
#endif

// special forms we need to recognize
#define EVAL_QUOTE  "quote"
#define EVAL_IF     "if"
#define EVAL_DEFINE "define"
#define EVAL_SET    "set!"
#define EVAL_LAMBDA "lambda"

static Cell* cell_quote(US* us, Cell* cell);  // no need for env
static Cell* cell_symbol(US* us, Cell* cell, Env* env);
static Cell* cell_apply(US* us, Cell* cell, Env* env);
static Cell* cell_apply_proc(US* us, Cell* cell, Env* env, Cell* proc);
static Cell* cell_apply_native(US* us, Cell* cell, Env* env, Cell* proc);
static Cell* cell_set_value(US* us, Cell* cell, Env* env, int create);
static Cell* cell_if(US* us, Cell* cell, Env* env);
static Cell* cell_lambda(US* us, Cell* cell, Env* env);
static int gather_args(Cell* cell, int wanted, Cell* args[]);

Cell* cell_eval(US* us, Cell* cell, Env* env)
{
    if (cell->tag == CELL_SYMBOL) {
        // a symbol => get it from the environment
        return cell_symbol(us, cell, env);
    }

    if (cell->tag != CELL_CONS) {
        // anything not a cons => self-evaluating
        return cell;
    }

    // we know for sure we have a cons cell
    Cell* car = cell->cons.car;
    LOG(DEBUG, ("EVAL: evaluating a cons cell, car is %s", cell_dump(car, 1, dumper)));

    // is it a special form?
    if (car->tag == CELL_SYMBOL) {
        if (strcmp(car->sval, EVAL_QUOTE) == 0) {
            // a quote special form
            return cell_quote(us, cell);
        }
        if (strcmp(car->sval, EVAL_DEFINE) == 0) {
            // a define special form
            return cell_set_value(us, cell, env, 1);
        }
        if (strcmp(car->sval, EVAL_SET) == 0) {
            // a set! special form
            return cell_set_value(us, cell, env, 0);
        }
        if (strcmp(car->sval, EVAL_IF) == 0) {
            // an if special form
            return cell_if(us, cell, env);
        }
        if (strcmp(car->sval, EVAL_LAMBDA) == 0 ) {
            // a lambda special form
            return cell_lambda(us, cell, env);
        }
    }

    // treat the cell as a function invocation
    return cell_apply(us, cell, env);
}

// Eval a symbol cell
static Cell* cell_symbol(US* us, Cell* cell, Env* env)
{
    (void) us;
    Cell* ret = nil;
    LOG(DEBUG, ("EVAL: looking up symbol [%s] in env %p", cell->sval, env));
    Symbol* sym = env_lookup(env, cell->sval, 0);
    if (sym) {
        ret = sym->value;
    }
    LOG(DEBUG, ("EVAL: looked up symbol [%s] in env %p => %s", cell->sval, env, cell_dump(ret, 1, dumper)));
    return ret;
}

// Execute a quote special form
static Cell* cell_quote(US* us, Cell* cell)
{
    (void) us;
    Cell* ret = nil;
    Cell* args[2];
    if (gather_args(cell, 2, args)) {
        LOG(DEBUG, ("EVAL: quote %s", cell_dump(args[1], 1, dumper)));
        ret = args[1];
    }
    return ret;
}

// Apply a function (car) to all its arguments (cdr)
static Cell* cell_apply(US* us, Cell* cell, Env* env)
{
    Cell* ret = 0;
    Cell* proc = cell_eval(us, cell->cons.car, env);
    if (proc) {
        switch (proc->tag) {
            case CELL_PROC:
                ret = cell_apply_proc(us, cell, env, proc);
                break;

            case CELL_NATIVE:
                ret = cell_apply_native(us, cell, env, proc);
                break;
        }
    }
    if (!ret) {
        ret = nil;
    }
    return ret;
}

static Cell* cell_apply_proc(US* us, Cell* cell, Env* env, Cell* proc)
{
    int pos = 0;
    Cell* p = 0; // pointer to current parameter
    Cell* a = 0; // pointer to current argument
    Cell* ret = 0;

    // count how many arguments we have
    for (p = proc->pval.params, a = cell->cons.cdr, pos = 0;
         p && p != nil && a && a != nil;
         p = p->cons.cdr, a = a->cons.cdr, ++pos) {
    }

    // We create a new small-ish environment where we can bind all evaled args
    // in fresh slots for the params (see *COMMENT* below)
    Env* local = arena_get_env(us->arena, pos + 1);
    LOG(DEBUG, ("EVAL: proc with %d args: %s", pos, cell_dump(proc, 1, dumper)));
    LOG(DEBUG, ("EVAL: proc on: %s", cell_dump(cell, 1, dumper)));
    int ok = 1;
    for (p = proc->pval.params, a = cell->cons.cdr, pos= 0;
         p && p != nil && a && a != nil;
         p = p->cons.cdr, a = a->cons.cdr, ++pos) {
        Cell* par = p->cons.car;
        if (!par) {
            LOG(ERROR, ("Could not get parameter #%d", pos));
            ok = 0;
            break;
        }
        LOG(DEBUG, ("Got parameter #%d: %s", pos, cell_dump(par, 1, dumper)));
        // we eval each arg in the caller's environment
        Cell* arg = cell_eval(us, a->cons.car, env);
        if (!arg) {
            LOG(ERROR, ("Could not evaluate arg #%d [%s]", pos, par->sval));
            ok = 0;
            break;
        }
        LOG(DEBUG, ("Evaluated arg #%d [%s]", pos, par->sval));
        // now we create a symbol with the correct name=value association
        Symbol* sym = env_lookup(local, par->sval, 1);
        if (!sym) {
            LOG(ERROR, ("Could not create binding for arg #%d [%s]", pos, par->sval));
            ok = 0;
            break;
        }
        sym->value = arg;
        LOG(DEBUG, ("Proc, setting arg #%d [%s] to %s", pos, par->sval, cell_dump(arg, 1, dumper)));
    }
    // *COMMENT* only *now* we chain our local env with its parent, which is
    // the env that we captured when the lamdba was created.
    env_chain(local, proc->pval.env);

    if (ok) {
        // finally eval the proc body in this newly created env
        ret = cell_eval(us, proc->pval.body, local);
    }
    if (!ret) {
        ret = nil;
    }
    return ret;
}

static Cell* cell_apply_native(US* us, Cell* cell, Env* env, Cell* proc)
{
    // We create a new list with all evaled args
    Cell* a = 0; // pointer to current argument
    Expression exp;
    LIST_RESET(exp);
    Cell* ret = 0;
    LOG(DEBUG, ("EVAL: native [%s] on %s", proc->nval.label, cell_dump(cell, 1, dumper)));
    int pos = 0;
    int ok = 1;
    for (a = cell->cons.cdr;
         a && a != nil;
         a = a->cons.cdr, ++pos) {
        // we eval each arg in the caller's environment
        Cell* arg = cell_eval(us, a->cons.car, env);
        if (!arg) {
            LOG(ERROR, ("Native, could not evaluate arg #%d for [%s]", pos, proc->nval.label));
            ok = 0;
            break;
        }
        Cell* cons = cell_cons(us, arg, nil);
        LIST_APPEND(&exp, cons);
        LOG(DEBUG, ("Native, arg #%d for [%s] is %s", pos, proc->nval.label, cell_dump(arg, 1, dumper)));
    }

    // finally eval the proc function with its args
    if (ok) {
        LOG(DEBUG, ("Native, calling with args %s", cell_dump(exp.frst, 1, dumper)));
        ret = proc->nval.func(us, exp.frst);
    }
    if (!ret) {
        ret = nil;
    }
    return ret;
}

// Set a value in the given environment;
// optionally create it if it doesn't exist.
static Cell* cell_set_value(US* us, Cell* cell, Env* env, int create)
{
    Cell* ret = 0;
    Cell* args[3];
    if (gather_args(cell, 3, args)) {
        LOG(DEBUG, ("EVAL: %s value for [%s] to %s", create ? "define" : "set", args[1]->sval, cell_dump(args[2], 1, dumper)));
        Symbol* sym = env_lookup(env, args[1]->sval, create);
        if (!sym) {
            LOG(ERROR, ("EVAL: symbol [%s] not found", args[1]->sval));
        } else {
            ret = cell_eval(us, args[2], env);
            sym->value = ret;
            LOG(DEBUG, ("Setting value [%s] to %s", args[1]->sval, cell_dump(ret, 1, dumper)));
        }
    }
    if (!ret) {
        ret = nil;
    }
    return ret;
}

// Execute an if special form
static Cell* cell_if(US* us, Cell* cell, Env* env)
{
    Cell* ret = 0;
    Cell* args[4];
    if (gather_args(cell, 4, args)) {
        LOG(DEBUG, ("EVAL: if Q %s", cell_dump(args[1], 1, dumper)));
        LOG(DEBUG, ("EVAL: if ? %s", cell_dump(args[2], 1, dumper)));
        LOG(DEBUG, ("EVAL: if : %s", cell_dump(args[3], 1, dumper)));

        Cell* tst = cell_eval(us, args[1], env);
        ret = cell_eval(us, tst == bool_t ? args[2] : args[3], env);
        LOG(DEBUG, ("EVAL: if => %s", cell_dump(ret, 1, dumper)));
    }
    if (!ret) {
        ret = nil;
    }
    return ret;
}

// Execute a lambda special form
static Cell* cell_lambda(US* us, Cell* cell, Env* env)
{
    Cell* ret = 0;
    Cell* args[3];
    if (gather_args(cell, 3, args)) {
        LOG(DEBUG, ("EVAL: lambda args %s", cell_dump(args[1], 1, dumper)));
        LOG(DEBUG, ("EVAL: lambda body %s", cell_dump(args[2], 1, dumper)));

        // This is where lexical scope happens: we keep the environment that
        // was extant at the time of the lambda *creation*, as opposed to its
        // *usage* (the latter would be dynamic scope).
        ret = cell_create_procedure(us, args[1], args[2], env);
    }
    if (!ret) {
        ret = nil;
    }
    return ret;
}

static int gather_args(Cell* cell, int wanted, Cell* args[])
{
    int pos = 0;
    for (Cell* c = cell; c && c != nil && pos < wanted; c = c->cons.cdr) {
        args[pos] = (Cell*) c->cons.car;
        if (!args[pos]) {
            return 0;
        }
        ++pos;
    }
    return pos == wanted;
}

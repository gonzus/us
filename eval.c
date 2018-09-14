#include <string.h>
#include "cell.h"
#include "env.h"
#include "eval.h"

// #define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"
#if LOG_LEVEL <= LOG_LEVEL_DEBUG
static char dumper[10*1024];
#endif

// special forms we need to recognize
#define EVAL_QUOTE  "quote"
#define EVAL_IF     "if"
#define EVAL_DEFINE "define"
#define EVAL_SET    "set!"
#define EVAL_LAMBDA "lambda"

static const Cell* cell_symbol(const Cell* cell, Env* env);
static const Cell* cell_quote(const Cell* cell);  // no need for env
static const Cell* cell_apply(const Cell* cell, Env* env);
static const Cell* cell_apply_proc(const Cell* cell, Env* env, const Cell* proc);
static const Cell* cell_apply_native(const Cell* cell, Env* env, const Cell* proc);
static const Cell* cell_set_value(const Cell* cell, Env* env, int create);
static const Cell* cell_if(const Cell* cell, Env* env);
static const Cell* cell_lambda(const Cell* cell, Env* env);
static int gather_args(const Cell* cell, int wanted, Cell* args[]);

const Cell* cell_eval(const Cell* cell, Env* env)
{
    if (cell->tag == CELL_SYMBOL) {
        // a symbol => get it from the environment
        return cell_symbol(cell, env);
    }

    if (cell->tag != CELL_CONS) {
        // anything not a cons => self-evaluating, just return it
        return cell;
    }


    // we know for sure we have a cons cell
    const Cell* car = cell->cons.car;
    LOG(DEBUG, ("EVAL: evaluating a cons cell, car is %s", cell_dump(car, 1, dumper)));

    // is it a special form?
    if (car->tag == CELL_SYMBOL) {
        if (strcmp(car->sval, EVAL_QUOTE) == 0) {
            // a quote special form
            return cell_quote(cell);
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

    // treat the cell as a function invocation
    return cell_apply(cell, env);
}

// Eval a symbol cell
static const Cell* cell_symbol(const Cell* cell, Env* env)
{
    const Cell* ret = nil;
    Symbol* symbol = env_lookup(env, cell->sval, 0);
    if (symbol) {
        ret = symbol->value;
    }
    LOG(DEBUG, ("EVAL: looked up symbol [%s] in env %p => %s", cell->sval, env, cell_dump(ret, 1, dumper)));
    return ret;
}

// Execute a quote special form
static const Cell* cell_quote(const Cell* cell)
{
    Cell* args[2];
    if (!gather_args(cell, 2, args)) {
        return nil;
    }

    LOG(DEBUG, ("EVAL: quote %s", cell_dump(args[1], 1, dumper)));
    return args[1];
}

// Apply a function (car) to all its arguments (cdr)
static const Cell* cell_apply(const Cell* cell, Env* env)
{
    const Cell* proc = cell_eval(cell->cons.car, env);
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

static const Cell* cell_apply_proc(const Cell* cell, Env* env, const Cell* proc)
{
    int pos = 0;
    const Cell* p = 0; // pointer to current parameter
    const Cell* a = 0; // pointer to current argument

    // count how many arguments we have
    for (p = proc->pval.params, a = cell->cons.cdr, pos = 0;
         p && p != nil && a && a != nil;
         p = p->cons.cdr, a = a->cons.cdr, ++pos) {
    }

    // We create a new small-ish environment where we can bind all evaled args
    // in fresh slots for the params (see *COMMENT* below)
    Env* local = env_create(pos + 1);
    env_ref(local);
    LOG(DEBUG, ("EVAL: proc on %s", cell_dump(cell, 1, dumper)));
    for (p = proc->pval.params, a = cell->cons.cdr, pos= 0;
         p && p != nil && a && a != nil;
         p = p->cons.cdr, a = a->cons.cdr, ++pos) {
        const Cell* par = p->cons.car;
        if (!par) {
            LOG(ERROR, ("Could not get parameter #%d", pos));
            return nil;
        }
        // we eval each arg in the caller's environment
        const Cell* arg = cell_eval(a->cons.car, env);
        if (!arg) {
            LOG(ERROR, ("Could not evaluate arg #%d [%s]", pos, par->sval));
            return nil;
        }
        // now we create a symbol with the correct name=value association
        Symbol* sym = env_lookup(local, par->sval, 1);
        if (!sym) {
            LOG(ERROR, ("Could not create binding for arg #%d [%s]", pos, par->sval));
            return nil;
        }
        sym->value = arg;
        LOG(DEBUG, ("Proc, setting arg #%d [%s] to %s", pos, par->sval, cell_dump(arg, 1, dumper)));
    }
    // *COMMENT* only *now* we chain our local env with its parent, which is
    // the env that we captured when the lamdba was created.
    env_chain(local, proc->pval.env);

    // finally eval the proc body in this newly created env
    const Cell* ret = cell_eval(proc->pval.body, local);

    // We cannot destroy our local env because it may have been "captured" and
    // will be returned to the caller; this  happens when returning a lambda as
    // the result of calling a procedure.
    // TODO: implement garbage collection :-)
    return ret;
}

static const Cell* cell_apply_native(const Cell* cell, Env* env, const Cell* proc)
{
    // We create a new list with all evaled args
    const Cell* a = 0; // pointer to current argument
    Cell* args = 0;
    Cell* last = 0;
    LOG(DEBUG, ("EVAL: native [%s] on %s", proc->nval.label, cell_dump(cell, 1, dumper)));
    int pos = 0;
    for (a = cell->cons.cdr;
         a && a != nil;
         a = a->cons.cdr, ++pos) {
        // we eval each arg in the caller's environment
        const Cell* arg = cell_eval(a->cons.car, env);
        if (!arg) {
            LOG(ERROR, ("Native, could not evaluate arg #%d for [%s]", pos, proc->nval.label));
            return nil;
        }
        Cell* cons = cell_cons(arg, nil);
        if (!args) {
            // first arg => remember it
            args = cons;
        }
        if (last) {
            // already have a last element => chain from it
            last->cons.cdr = cons;
        }
        // remember last element
        last = cons;
        LOG(DEBUG, ("Native, arg #%d for [%s] is %s", pos, proc->nval.label, cell_dump(arg, 1, dumper)));
    }

    // finally eval the proc function with its args
    const Cell* ret = proc->nval.func(args);

    return ret;
}

// Set a value in the given environment;
// optionally create it if it doesn't exist.
static const Cell* cell_set_value(const Cell* cell, Env* env, int create)
{
    Cell* args[3];
    if (!gather_args(cell, 3, args)) {
        return nil;
    }

    LOG(DEBUG, ("EVAL: %s value for [%s] to %s", create ? "define" : "set", args[1]->sval, cell_dump(args[2], 1, dumper)));
    Symbol* symbol = env_lookup(env, args[1]->sval, create);
    if (!symbol) {
        LOG(ERROR, ("EVAL: symbol [%s] not found", args[1]->sval));
        return nil;
    }

    const Cell* ret = cell_eval(args[2], env);
    symbol->value = ret;
    LOG(DEBUG, ("Setting value [%s] to %s", args[1]->sval, cell_dump(ret, 1, dumper)));
    return ret;
}

// Execute an if special form
static const Cell* cell_if(const Cell* cell, Env* env)
{
    Cell* args[4];
    if (!gather_args(cell, 4, args)) {
        return nil;
    }

    LOG(DEBUG, ("EVAL: if Q %s", cell_dump(args[1], 1, dumper)));
    LOG(DEBUG, ("EVAL: if ? %s", cell_dump(args[2], 1, dumper)));
    LOG(DEBUG, ("EVAL: if : %s", cell_dump(args[3], 1, dumper)));

    const Cell* tst = cell_eval(args[1], env);
    const Cell* ans = cell_eval(tst == bool_t ? args[2] : args[3], env);
    LOG(DEBUG, ("EVAL: if => %s", cell_dump(ans, 1, dumper)));
    return ans;
}

// Execute a lambda special form
static const Cell* cell_lambda(const Cell* cell, Env* env)
{
    Cell* args[3];
    if (!gather_args(cell, 3, args)) {
        return nil;
    }

    LOG(DEBUG, ("EVAL: lambda args %s", cell_dump(args[1], 1, dumper)));
    LOG(DEBUG, ("EVAL: lambda body %s", cell_dump(args[2], 1, dumper)));

    // This is where lexical scope happens: we keep the environment that was
    // extant at the time of the lambda *creation*, as opposed to its *usage*
    // (the latter would be dynamic scope).
    return cell_create_procedure(args[1], args[2], env);
}

static int gather_args(const Cell* cell, int wanted, Cell* args[])
{
    int pos = 0;
    for (const Cell* c = cell; c && c != nil && pos < wanted; c = c->cons.cdr) {
        args[pos] = (Cell*) c->cons.car;
        if (!args[pos]) {
            return 0;
        }
        ++pos;
    }
    return pos == wanted;
}

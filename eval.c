#include <string.h>
#include "cell.h"
#include "env.h"
#include "parser.h"
#include "eval.h"

// #define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"
#if LOG_LEVEL <= LOG_LEVEL_INFO
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
        return cell;
        // anything not a cons => self-evaluating
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
    Symbol* sym = env_lookup(env, cell->sval, 0);
    if (sym) {
        ret = sym->value;
    }
    return ret;
    LOG(INFO, ("EVAL: looked up symbol [%s] in env %p => %s", cell->sval, env, cell_dump(ret, 1, dumper)));
}

// Execute a quote special form
static const Cell* cell_quote(const Cell* cell)
{
    Cell* ret = nil;
    Cell* args[2];
    if (gather_args(cell, 2, args)) {
        LOG(DEBUG, ("EVAL: quote %s", cell_dump(args[1], 1, dumper)));
        ret = args[1];
    }
    return ret;
}

// Apply a function (car) to all its arguments (cdr)
static const Cell* cell_apply(const Cell* cell, Env* env)
{
    Cell* ret = 0;
    const Cell* proc = cell_eval(cell->cons.car, env);
    if (proc) {
        switch (proc->tag) {
            case CELL_PROC:
                ret = cell_apply_proc(cell, env, proc);
                break;

            case CELL_NATIVE:
                ret = cell_apply_native(cell, env, proc);
                break;
        }
    }
    if (!ret) {
        ret = nil;
    }
    return ret;
}

static const Cell* cell_apply_proc(const Cell* cell, Env* env, const Cell* proc)
{
    int pos = 0;
    const Cell* p = 0; // pointer to current parameter
    const Cell* a = 0; // pointer to current argument
    Cell* ret = 0;

    // count how many arguments we have
    for (p = proc->pval.params, a = cell->cons.cdr, pos = 0;
         p && p != nil && a && a != nil;
         p = p->cons.cdr, a = a->cons.cdr, ++pos) {
    }

    // We create a new small-ish environment where we can bind all evaled args
    // in fresh slots for the params (see *COMMENT* below)
    Env* local = env_ref(env_create(pos + 1));
    LOG(INFO, ("EVAL: proc on %s", cell_dump(cell, 1, dumper)));
    int ok = 1;
    for (p = proc->pval.params, a = cell->cons.cdr, pos= 0;
         p && p != nil && a && a != nil;
         p = p->cons.cdr, a = a->cons.cdr, ++pos) {
        const Cell* par = p->cons.car;
        if (!par) {
            LOG(ERROR, ("Could not get parameter #%d", pos));
            ok = 0;
            break;
        }
        // we eval each arg in the caller's environment
        const Cell* arg = cell_eval(a->cons.car, env);
        if (!arg) {
            LOG(ERROR, ("Could not evaluate arg #%d [%s]", pos, par->sval));
            ok = 0;
            break;
        }
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
        ret = cell_eval(proc->pval.body, local);

        // We cannot destroy our local env because it may have been "captured" and
        // will be returned to the caller; this  happens when returning a lambda as
        // the result of calling a procedure.
        // TODO: implement garbage collection :-)
    }
    if (!ret) {
        ret = nil;
    }
    return ret;
}

static const Cell* cell_apply_native(const Cell* cell, Env* env, const Cell* proc)
{
    // We create a new list with all evaled args
    const Cell* a = 0; // pointer to current argument
    Expression exp;
    PARSER_EXP_RESET(exp);
    Cell* ret = 0;
    LOG(INFO, ("EVAL: native [%s] on %s", proc->nval.label, cell_dump(cell, 1, dumper)));
    int pos = 0;
    int ok = 1;
    for (a = cell->cons.cdr;
         a && a != nil;
         a = a->cons.cdr, ++pos) {
        // we eval each arg in the caller's environment
        const Cell* arg = cell_eval(a->cons.car, env);
        if (!arg) {
            LOG(ERROR, ("Native, could not evaluate arg #%d for [%s]", pos, proc->nval.label));
            ok = 0;
            break;
        }
        Cell* cons = cell_cons(arg, nil);
        PARSER_EXP_APPEND(&exp, cons);
        LOG(DEBUG, ("Native, arg #%d for [%s] is %s", pos, proc->nval.label, cell_dump(arg, 1, dumper)));
    }

    // finally eval the proc function with its args
    if (ok) {
        ret = proc->nval.func(exp.frst);
    }
    if (!ret) {
        ret = nil;
    }
    LOG(INFO, ("native: returning"));
    return ret;
}

// Set a value in the given environment;
// optionally create it if it doesn't exist.
static const Cell* cell_set_value(const Cell* cell, Env* env, int create)
{
    Cell* ret = 0;
    Cell* args[3];
    if (gather_args(cell, 3, args)) {
        LOG(DEBUG, ("EVAL: %s value for [%s] to %s", create ? "define" : "set", args[1]->sval, cell_dump(args[2], 1, dumper)));
        Symbol* sym = env_lookup(env, args[1]->sval, create);
        if (!sym) {
            LOG(ERROR, ("EVAL: symbol [%s] not found", args[1]->sval));
        }
        else {
            ret = cell_eval(args[2], env);
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
static const Cell* cell_if(const Cell* cell, Env* env)
{
    Cell* ret = 0;
    Cell* args[4];
    if (gather_args(cell, 4, args)) {
        LOG(DEBUG, ("EVAL: if Q %s", cell_dump(args[1], 1, dumper)));
        LOG(DEBUG, ("EVAL: if ? %s", cell_dump(args[2], 1, dumper)));
        LOG(DEBUG, ("EVAL: if : %s", cell_dump(args[3], 1, dumper)));

        const Cell* tst = cell_eval(args[1], env);
        ret = cell_eval(tst == bool_t ? args[2] : args[3], env);
        LOG(DEBUG, ("EVAL: if => %s", cell_dump(ret, 1, dumper)));
    }
    if (!ret) {
        ret = nil;
    }
    return ret;
}

// Execute a lambda special form
static const Cell* cell_lambda(const Cell* cell, Env* env)
{
    Cell* ret = 0;
    Cell* args[3];
    if (gather_args(cell, 3, args)) {
        LOG(INFO, ("EVAL: lambda args %s", cell_dump(args[1], 1, dumper)));
        LOG(INFO, ("EVAL: lambda body %s", cell_dump(args[2], 1, dumper)));

        // This is where lexical scope happens: we keep the environment that was
        // extant at the time of the lambda *creation*, as opposed to its *usage*
        // (the latter would be dynamic scope).
        ret = cell_create_procedure(args[1], args[2], env);
    }
    if (!ret) {
        ret = nil;
    }
    return ret;
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

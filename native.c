#include <stdio.h>
#include <string.h>
#include "cell.h"
#include "native.h"

// #define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"
#if LOG_LEVEL <= LOG_LEVEL_DEBUG
static char dumper[10*1024];
#endif

#define CELL_LOOP(name, pos, args, body) \
    do { \
        LOG(DEBUG, ("Entering native %s", name)); \
        for (Cell* c = args; c && c != nil; c = c->cons.cdr, ++pos) { \
            Cell* arg = c->cons.car; \
            LOG(DEBUG, ("Arg #%d %s", pos, cell_dump(arg, dumper))); \
            do body while (0); \
        } \
        LOG(DEBUG, ("Leaving native %s", name)); \
    } while (0)

Cell* func_add(Cell* args)
{
    long iret = 0;
    double rret = 0.0;
    int isaw = 0;
    int rsaw = 0;
    int ok = 1;
    int pos = 0;
    CELL_LOOP("add", pos, args, {
        switch (arg->tag) {
            case CELL_INT : isaw = 1; iret += arg->ival; break;
            case CELL_REAL: rsaw = 1; rret += arg->rval; break;
            default: ok = 0; break;
        }
        if (!ok) break;
    });
    if (!ok) return nil;
    if (rsaw) return cell_create_real(rret + iret);
    if (isaw) return cell_create_int(iret);
    return nil;
}

Cell* func_sub(Cell* args)
{
    long iret = 0;
    double rret = 0.0;
    int isaw = 0;
    int rsaw = 0;
    int ok = 1;
    int pos = 0;
    CELL_LOOP("sub", pos, args, {
        if (pos == 0) {
            switch (arg->tag) {
                case CELL_INT : isaw = 1; iret = arg->ival; break;
                case CELL_REAL: rsaw = 1; rret = arg->rval; break;
                default: ok = 0; break;
            }
        }
        else {
            switch (arg->tag) {
                case CELL_INT : isaw = 1; iret -= arg->ival; break;
                case CELL_REAL: rsaw = 1; rret -= arg->rval; break;
                default: ok = 0; break;
            }
        }
        if (!ok) break;
    });
    if (!ok) return nil;
    if (pos == 0) return nil;
    if (pos == 1) { iret = -iret; rret = -rret; }
    if (rsaw) return cell_create_real(rret + iret);
    if (isaw) return cell_create_int(iret);
    return nil;
}

Cell* func_mul(Cell* args)
{
    long iret = 1;
    double rret = 1.0;
    int isaw = 0;
    int rsaw = 0;
    int ok = 1;
    int pos = 0;
    CELL_LOOP("mul", pos, args, {
        switch (arg->tag) {
            case CELL_INT : isaw = 1; iret *= arg->ival; break;
            case CELL_REAL: rsaw = 1; rret *= arg->rval; break;
            default: ok = 0; break;
        }
        if (!ok) break;
    });
    if (!ok) return nil;
    if (rsaw) return cell_create_real(rret * iret);
    if (isaw) return cell_create_int(iret);
    return nil;
}

Cell* func_div(Cell* args)
{
    long iret = 0;
    double rret = 0.0;
    int isaw = 0;
    int rsaw = 0;
    int ok = 1;
    int pos = 0;
    CELL_LOOP("div", pos, args, {
        if (pos == 0) {
            switch (arg->tag) {
                case CELL_INT : isaw = 1; iret = arg->ival; break;
                case CELL_REAL: rsaw = 1; rret = arg->rval; break;
                default: ok = 0; break;
            }
            continue;
        }
        switch (arg->tag) {
            case CELL_INT :
                if (arg->ival == 0) {
                    ok = 0;
                    break;
                }
                if (isaw) {
                    long tmp = iret / arg->ival;
                    if ((tmp * arg->ival) == iret) {
                        // exact long division
                        iret = tmp;
                        break;
                    }
                    else {
                        // switch to using reals
                        rret = (double) iret / (double) arg->ival;
                        isaw = 0;
                        rsaw = 1;
                    }
                }
                else {
                    rret /= (double) arg->ival;
                }
                break;

            case CELL_REAL:
                if (arg->rval == 0.0) {
                    ok = 0;
                    break;
                }
                if (isaw) {
                    // switch to using reals
                    rret = iret;
                    isaw = 0;
                    rsaw = 1;
                }
                rret /= arg->rval;
                break;

            default: ok = 0; break;
        }
        if (!ok) break;
    });
    LOG(DEBUG, ("DIV: pos %d, ok %d, isaw %d, rsaw %d, iret %ld, rret %lf", pos, ok, isaw, rsaw, iret, rret));
    if (pos == 0) return nil;
    if (pos == 1) {
        if (isaw) {
            if (iret == 0) {
                ok = 0;
            }
            else if (iret == 1 || iret == -1) {
                // 1/n == n when n == 1 or n == -1; do nothing
            }
            else {
                // switch to using reals
                rret = 1.0 / (double) iret;
                isaw = 0;
                rsaw = 1;
            }
        } else {
            if (rret == 0.0) {
                ok = 0;
            }
            else {
                rret = 1.0 / rret;
            }
        }
    }
    if (!ok) return nil;
    if (rsaw) return cell_create_real(rret);
    if (isaw) return cell_create_int(iret);
    return nil;
}

Cell* func_eq(Cell* args)
{
    int ok = 1;
    int pos = 0;
    Cell* mem = 0;
    CELL_LOOP("eq", pos, args, {
        if (!pos) { mem = arg; continue; }
        if (mem->tag != arg->tag) { ok = 0; break; }
        switch (mem->tag) {
            case CELL_NONE  : break;
            case CELL_INT   : ok = mem->ival == arg->ival; break;
            case CELL_REAL  : ok = mem->rval == arg->rval; break;
            case CELL_STRING: // fall through
            case CELL_SYMBOL: ok = strcmp(mem->sval, arg->sval) == 0; break;
            case CELL_NATIVE: ok = mem->nval.func == arg->nval.func; break;
            default: ok = 0; break;
        }
        if (!ok) { break; }
    });
    LOG(DEBUG, ("EQ => %d", ok));
    return ok ? bool_t : bool_f;
}

Cell* func_gt(Cell* args)
{
    int ok = 1;
    int pos = 0;
    Cell* mem = 0;
    CELL_LOOP("gt", pos, args, {
        if (!pos) { mem = arg; continue; }
        if (mem->tag != arg->tag) { ok = 0; break; }
        switch (mem->tag) {
            case CELL_INT   : ok = mem->ival > arg->ival; break;
            case CELL_REAL  : ok = mem->rval > arg->rval; break;
            case CELL_STRING: // fall through
            case CELL_SYMBOL: ok = strcmp(mem->sval, arg->sval) > 0; break;
            default: ok = 0; break;
        }
        if (!ok) { break; }
        mem = arg;
    });
    LOG(DEBUG, ("GT => %d", ok));
    return ok ? bool_t : bool_f;
}

Cell* func_lt(Cell* args)
{
    int ok = 1;
    int pos = 0;
    Cell* mem = 0;
    CELL_LOOP("lt", pos, args, {
        if (!pos) { mem = arg; continue; }
        if (mem->tag != arg->tag) { ok = 0; break; }
        switch (mem->tag) {
            case CELL_INT   : ok = mem->ival < arg->ival; break;
            case CELL_REAL  : ok = mem->rval < arg->rval; break;
            case CELL_STRING: // fall through
            case CELL_SYMBOL: ok = strcmp(mem->sval, arg->sval) < 0; break;
            default: ok = 0; break;
        }
        if (!ok) { break; }
        mem = arg;
    });
    LOG(DEBUG, ("LT => %d", ok));
    return ok ? bool_t : bool_f;
}

Cell* func_cons(Cell* args)
{
    Cell* ret = nil;
    Cell* mem[2];
    int pos = 0;
    CELL_LOOP("cons", pos, args, {
        if (pos >= 2) break;
        mem[pos] = arg;
    });
    if (pos == 2) {
        ret = cell_cons(mem[0], mem[1]);
    }
    LOG(DEBUG, ("CONS: %s", cell_dump(ret, dumper)));
    return ret;
}

Cell* func_car(Cell* args)
{
    Cell* ret = nil;
    Cell* mem[1];
    int pos = 0;
    CELL_LOOP("car", pos, args, {
        if (pos >= 1) break;
        mem[pos] = arg;
    });
    if (pos == 1) {
        ret = cell_car(mem[0]);
    }
    LOG(DEBUG, ("CAR: %s", cell_dump(ret, dumper)));
    return ret;
}

Cell* func_cdr(Cell* args)
{
    Cell* ret = nil;
    Cell* mem[1];
    int pos = 0;
    CELL_LOOP("cdr", pos, args, {
        if (pos >= 1) break;
        mem[pos] = arg;
    });
    if (pos == 1) {
        ret = cell_cdr(mem[0]);
    }
    LOG(DEBUG, ("CDR: %s", cell_dump(ret, dumper)));
    return ret;
}


Cell* func_begin(Cell* args)
{
    // we don't really do anything here, except remember the last value
    Cell* ret = nil;
    int pos = 0;
    CELL_LOOP("begin", pos, args, {
        ret = arg;
    });
    return ret;
}

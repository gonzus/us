#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "cell.h"

#if !defined(MEM_DEBUG)
#define MEM_DEBUG 0
#endif
#include "mem.h"

// #define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"

// These are special values that have a single unique instance
static Cell cell_nil    = { CELL_NONE, {0} };
static Cell cell_bool_t = { CELL_INT , {1} };
static Cell cell_bool_f = { CELL_INT , {0} };

// Global references to the special values
const Cell* nil    = &cell_nil;
const Cell* bool_t = &cell_bool_t;
const Cell* bool_f = &cell_bool_f;

static Cell* cell_build(int tag);
static int get_str_len(const char* str, int len);
static int cell_printer(const Cell* cell, int debug, char* buf);
static int cell_print_all(const Cell* cell, char* buf);

void cell_destroy(Cell* cell)
{
    LOG(INFO, ("CELL: destroying %p tag %d", cell, cell->tag));
    switch (cell->tag) {
        case CELL_STRING:
        case CELL_SYMBOL:
            MEM_FREE_SIZE(cell->sval, 0);
            break;
        case CELL_CONS:
#if 0
            cell_unref(cell->cons.car);
            cell_unref(cell->cons.cdr);
#endif
            break;
        case CELL_PROC:
#if 0
            cell_unref(cell->pval.params);
            cell_unref(cell->pval.body);
            env_unref(cell->pval.env);
#endif
            break;
    }
    MEM_FREE_TYPE(cell, 1, Cell);
}

Cell* cell_create_int(long value)
{
    Cell* cell = cell_build(CELL_INT);
    cell->ival = value;
    char dumper[10*1024];
    LOG(INFO, ("CELL: created %p [%s]", cell, cell_dump(cell, 1, dumper)));
    return cell;
}

Cell* cell_create_int_from_string(const char* value, int len)
{
    len = get_str_len(value, len);
    int sign = 1;
    long lval = 0;
    for (int j = 0; j < len; ++j) {
        if (value[j] == '+') {
            sign = 1;
            continue;
        }
        if (value[j] == '-') {
            sign = -1;
            continue;
        }
        int digit = value[j] - '0';
        lval = lval * 10 + digit;
    }
    return cell_create_int(sign * lval);
}

Cell* cell_create_real(double value)
{
    Cell* cell = cell_build(CELL_REAL);
    cell->rval = value;
    char dumper[10*1024];
    LOG(INFO, ("CELL: created %p [%s]", cell, cell_dump(cell, 1, dumper)));
    return cell;
}

Cell* cell_create_real_from_string(const char* value, int len)
{
    len = get_str_len(value, len);
    int sign = 1;
    int decs = 0;
    double dval = 0.0;
    for (int j = 0; j < len; ++j) {
        if (value[j] == '+') {
            sign = 1;
            continue;
        }
        if (value[j] == '-') {
            sign = -1;
            continue;
        }
        if (value[j] == '.') {
            decs = 1;
            continue;
        }
        int digit = value[j] - '0';
        if (decs) {
            decs *= 10;
            dval += (double) digit / decs;
        } else {
            dval = dval * 10.0 + digit;
        }
    }
    return cell_create_real(sign * dval);
}

Cell* cell_create_string(const char* value, int len)
{
    Cell* cell = cell_build(CELL_STRING);
    len = get_str_len(value, len);
    MEM_ALLOC_SIZE(cell->sval, len + 1);
    if (value && len) {
        memcpy(cell->sval, value, len);
    }
    cell->sval[len] = '\0';
    char dumper[10*1024];
    LOG(INFO, ("CELL: created %p [%s]", cell, cell_dump(cell, 1, dumper)));
    return cell;
}

Cell* cell_create_symbol(const char* value, int len)
{
    Cell* cell = cell_create_string(value, len);
    cell->tag = CELL_SYMBOL;
    char dumper[10*1024];
    LOG(INFO, ("CELL: created %p [%s]", cell, cell_dump(cell, 1, dumper)));
    return cell;
}

Cell* cell_create_procedure(const Cell* params, const Cell* body, Env* env)
{
    Cell* cell = cell_build(CELL_PROC);
    cell->pval.params = params;
    cell->pval.body = body;
    cell->pval.env = env;  // I love you, lexical binding
    char dumper[10*1024];
    LOG(INFO, ("CELL: created %p [%s]", cell, cell_dump(cell, 1, dumper)));
    return cell;
}

Cell* cell_create_native(const char* label, NativeFunc* func)
{
    Cell* cell = cell_build(CELL_NATIVE);
    cell->nval.label = label;
    cell->nval.func = func;
    char dumper[10*1024];
    LOG(DEBUG, ("CELL: created %p [%s]", cell, cell_dump(cell, 1, dumper)));
    return cell;
}

Cell* cell_cons(const Cell* car, const Cell* cdr)
{
    Cell* cell = cell_build(CELL_CONS);
    cell->cons.car = car;
    cell->cons.cdr = cdr;
    char dumper[10*1024];
    LOG(INFO, ("CELL: created %p [%s]", cell, cell_dump(cell, 1, dumper)));
    return cell;
}

const Cell* cell_car(const Cell* cell)
{
    if (cell->tag != CELL_CONS) {
        return 0;
    }
    return cell->cons.car;
}

const Cell* cell_cdr(const Cell* cell)
{
    if (cell->tag != CELL_CONS) {
        return 0;
    }
    return cell->cons.cdr;
}

void cell_print(const Cell* cell, FILE* fp, int eol)
{
    char buf[10 * 1024];
    cell_printer(cell, 0, buf);
    fputs(buf, fp);
    if (eol) {
        fputc('\n', fp);
    }
    fflush(fp);
}

const char* cell_dump(const Cell* cell, int debug, char* buf)
{
    cell_printer(cell, debug, buf);
    return buf;
}

static Cell* cell_build(int tag)
{
    Cell* cell = 0;
    MEM_ALLOC_TYPE(cell, 1, Cell);
    cell->tag = tag;
    return cell;
}

static int get_str_len(const char* str, int len)
{
    if (!str || str[0] == '\0') {
        len = 0;
    }
    if (len == 0 && str && str[0] != '\0') {
        len = strlen(str);
    }
    return len;
}

static int cell_printer(const Cell* cell, int debug, char* buf)
{
    static char* Tag[CELL_LAST] = {
        "NONE",
        "INT",
        "REAL",
        "STRING",
        "SYMBOL",
        "CONS",
        "PROC",
        "NATIVE",
    };
    int pos = 0;

    if (debug) {
        pos += sprintf(buf + pos, "cell[");
    }
    if (!cell) {
        pos += sprintf(buf + pos, "NULL");
    }
    else {
        if (debug) {
            const char* str = "???";
            if (cell->tag < CELL_LAST) {
                str = Tag[cell->tag];
            }
            pos += sprintf(buf + pos, "%d:%s:%p", cell->tag, str, cell);
            if (cell->tag != CELL_NONE) {
                pos += sprintf(buf + pos, ":");
            }
        }
        if (cell->tag != CELL_CONS) {
            pos += cell_print_all(cell, buf + pos);
        }
        else {
            pos += sprintf(buf + pos, "(");
            pos += cell_print_all(cell, buf + pos);
            pos += sprintf(buf + pos, ")");
        }
    }
    if (debug) {
        pos += sprintf(buf + pos, "]");
    }
    return pos;
}

static int cell_print_all(const Cell* cell, char* buf)
{
    int pos = 0;

    if (cell == nil) {
        pos += sprintf(buf + pos, "%s", CELL_STR_NIL);
        return pos;
    }
    if (cell == bool_t) {
        pos += sprintf(buf + pos, "%s", CELL_STR_BOOL_T);
        return pos;
    }
    if (cell == bool_f) {
        pos += sprintf(buf + pos, "%s", CELL_STR_BOOL_F);
        return pos;
    }

    switch (cell->tag) {
        case CELL_NONE:
            pos += sprintf(buf + pos, "%s", CELL_STR_NIL);
            break;

        case CELL_INT:
            pos += sprintf(buf + pos, "%ld", cell->ival);
            break;

        case CELL_REAL:
            pos += sprintf(buf + pos, "%lf", cell->rval);
            break;

        case CELL_STRING:
            pos += sprintf(buf + pos, "\"%s\"", cell->sval);
            break;

        case CELL_SYMBOL:
            pos += sprintf(buf + pos, "%s", cell->sval);
            break;

        case CELL_PROC: {
#if 1
            pos += sprintf(buf + pos, "<%s>", "*CODE*");
#else
            const Procedure* proc = &cell->pval;
            pos += sprintf(buf + pos, "(");
            pos += cell_print_all(proc->params, buf + pos);
            pos += sprintf(buf + pos, "):(");
            pos += cell_print_all(proc->body, buf + pos);
            pos += sprintf(buf + pos, ")");
#endif
            break;
        }

        case CELL_NATIVE:
            pos += sprintf(buf + pos, "<%s>", cell->nval.label);
            break;

        case CELL_CONS: {
            const Cons* cons = &cell->cons;
            if (cons->car->tag == CELL_CONS) {
                pos += sprintf(buf + pos, "(");
                pos += cell_print_all(cons->car, buf + pos);
                pos += sprintf(buf + pos, ")");
            }
            else {
                pos += cell_print_all(cons->car, buf + pos);
            }

            if (cons->cdr == nil) {
                // end of list, nothing else to do
            }
            else if (cons->cdr->tag == CELL_CONS) {
                pos += sprintf(buf + pos, " ");
                pos += cell_print_all(cons->cdr, buf + pos);
            }
            else {
                pos += sprintf(buf + pos, " . ");
                pos += cell_print_all(cons->cdr, buf + pos);
            }
            break;
        }
    }

    return pos;
}

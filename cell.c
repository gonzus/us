#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "cell.h"

// These are special values that have a single unique instance
static Cell cell_nil    = { CELL_NONE, {0} };
static Cell cell_bool_t = { CELL_INT , {1} };
static Cell cell_bool_f = { CELL_INT , {0} };

Cell* nil    = &cell_nil;
Cell* bool_t = &cell_bool_t;
Cell* bool_f = &cell_bool_f;

static int get_str_len(const char* str, int len);
static int cell_printer(const Cell* cell, int dump, char* buf);
static int cell_print_all(const Cell* cell, char* buf);

void cell_destroy(Cell* cell)
{
    switch (cell->tag) {
        case CELL_STRING:
        case CELL_SYMBOL:
            free((void*) cell->sval);
            break;
    }
    free(cell);
}

Cell* cell_create_int(long value)
{
    Cell* cell = (Cell*) malloc(sizeof(Cell));
    cell->tag = CELL_INT;
    cell->ival = value;
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
    Cell* cell = (Cell*) malloc(sizeof(Cell));
    cell->tag = CELL_REAL;
    cell->rval = value;
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
    Cell* cell = (Cell*) malloc(sizeof(Cell));
    cell->tag = CELL_STRING;

    len = get_str_len(value, len);
    cell->sval = malloc(len + 1);
    memcpy(cell->sval, value ? value : "", len);
    cell->sval[len] = '\0';

    return cell;
}

Cell* cell_create_symbol(const char* value, int len)
{
    Cell* cell = cell_create_string(value, len);
    cell->tag = CELL_SYMBOL;

    return cell;
}

Cell* cell_create_procedure(Cell* params, Cell* body, Env* env)
{
    Cell* cell = (Cell*) malloc(sizeof(Cell));
    cell->tag = CELL_PROC;
    cell->pval.params = params;
    cell->pval.body = body;
    cell->pval.env = env;  // I love you, lexical binding

    return cell;
}

Cell* cell_create_native(const char* label, NativeFunc* func)
{
    Cell* cell = (Cell*) malloc(sizeof(Cell));
    cell->tag = CELL_NATIVE;
    cell->nval.label = label;
    cell->nval.func = func;

    return cell;
}


// TODO: these three functions (and maybe others) should do
// some checking for their args...
Cell* cell_cons(Cell* car, Cell* cdr)
{
    Cell* cell = (Cell*) malloc(sizeof(Cell));
    cell->tag = CELL_CONS;
    cell->cons.car = car;
    cell->cons.cdr = cdr;
    return cell;
}

Cell* cell_car(const Cell* cell)
{
    if (cell->tag != CELL_CONS) {
        return 0;
    }
    return cell->cons.car;
}

Cell* cell_cdr(const Cell* cell)
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

const char* cell_dump(const Cell* cell, char* buf)
{
    cell_printer(cell, 1, buf);
    return buf;
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

static int cell_printer(const Cell* cell, int dump, char* buf)
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

    if (dump) {
        pos += sprintf(buf + pos, "cell[");
    }
    if (!cell) {
        pos += sprintf(buf + pos, "NULL");
    }
    else {
        if (dump) {
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
    if (dump) {
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

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
static void cell_printer(const Cell* cell, FILE* fp, int dump, int eol);
static void cell_print_all(const Cell* cell, FILE* fp);

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
    cell_printer(cell, fp, 0, eol);
}

void cell_dump(const Cell* cell, FILE* fp, int eol)
{
    cell_printer(cell, fp, 1, eol);
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

static void cell_printer(const Cell* cell, FILE* fp, int dump, int eol)
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

    if (dump) {
        fputs("cell<", fp);
    }
    if (!cell) {
        fputs("NULL", fp);
    }
    else {
        if (dump) {
            const char* str = "???";
            if (cell->tag < CELL_LAST) {
                str = Tag[cell->tag];
            }
            fprintf(fp, "%d:%s:%p", cell->tag, str, cell);
            if (cell->tag != CELL_NONE) {
                fputc(':', fp);
            }
        }
        if (cell->tag != CELL_CONS) {
            cell_print_all(cell, fp);
        }
        else {
            fputc('(', fp);
            cell_print_all(cell, fp);
            fputc(')', fp);
        }
    }
    if (dump) {
        fputs(">", fp);
    }
    if (eol) {
        fputc('\n', fp);
    }
}

static void cell_print_all(const Cell* cell, FILE* fp)
{
    if (cell == nil) {
        fputs(CELL_STR_NIL, fp);
        return;
    }
    if (cell == bool_t) {
        fputs(CELL_STR_BOOL_T, fp);
        return;
    }
    if (cell == bool_f) {
        fputs(CELL_STR_BOOL_F, fp);
        return;
    }

    switch (cell->tag) {
        case CELL_NONE:
            fputs(CELL_STR_NIL, fp);
            break;

        case CELL_INT:
            fprintf(fp, "%ld", cell->ival);
            break;

        case CELL_REAL:
            fprintf(fp, "%lf", cell->rval);
            break;

        case CELL_STRING:
            fprintf(fp, "\"%s\"", cell->sval);
            break;

        case CELL_SYMBOL:
            fprintf(fp, "%s", cell->sval);
            break;

        case CELL_PROC:
            fprintf(fp, "<%s>", "*CODE*");
            break;

        case CELL_NATIVE:
            fprintf(fp, "<%s>", cell->nval.label);
            break;

        case CELL_CONS: {
            const Cons* cons = &cell->cons;
            if (cons->car->tag == CELL_CONS) {
                fputc('(', fp);
                cell_print_all(cons->car, fp);
                fputc(')', fp);
            }
            else {
                cell_print_all(cons->car, fp);
            }

            if (cons->cdr == nil) {
                // end of list, nothing else to do
            }
            else if (cons->cdr->tag == CELL_CONS) {
                fputc(' ', fp);
                cell_print_all(cons->cdr, fp);
            }
            else {
                fputs(" . ", fp);
                cell_print_all(cons->cdr, fp);
            }
            break;
        }
    }
}

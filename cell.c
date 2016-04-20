#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cell.h"

#define CELL_NIL "nil"

static Cell cell_nil;
Cell* nil = &cell_nil;

void cell_destroy(Cell* cell)
{
    switch (cell->tag) {
        case CELL_STRING:
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

static int get_length(const char* value, int len)
{
    if (!value || value[0] == '\0') {
        len = 0;
    }
    if (len == 0 && value && value[0] != '\0') {
        len = strlen(value);
    }
    return len;
}

Cell* cell_create_int_from_string(const char* value, int len)
{
    len = get_length(value, len);
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
    len = get_length(value, len);
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

    len = get_length(value, len);
    cell->sval = malloc(len + 1);
    memcpy(cell->sval, value ? value : "", len);
    cell->sval[len] = '\0';

    return cell;
}

Cell* cell_create_symbol(const char* value, int len)
{
    Cell* cell = (Cell*) malloc(sizeof(Cell));
    cell->tag = CELL_SYMBOL;

    len = get_length(value, len);
    cell->sval = malloc(len + 1);
    memcpy(cell->sval, value ? value : "", len);
    cell->sval[len] = '\0';

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
    return cell->cons.car;
}

Cell* cell_cdr(const Cell* cell)
{
    return cell->cons.cdr;
}

static void cell_print_all(const Cell* cell, FILE* fp)
{
    switch (cell->tag) {
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

        case CELL_CONS: {
            const Cons* cons = &cell->cons;
            if (cons->car == nil) {
                fputs(CELL_NIL, fp);
            }
            else if (cons->car->tag == CELL_CONS) {
                fputc('(', fp);
                cell_print_all(cons->car, fp);
                fputc(')', fp);
            }
            else {
                cell_print_all(cons->car, fp);
            }

            if (cons->cdr == nil) {
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

void cell_print(const Cell* cell, FILE* fp, int eol)
{
    if (cell == nil) {
        fputs(CELL_NIL, fp);
    }
    else {
       if (cell->tag == CELL_CONS) {
           fputc('(', fp);
       }
       cell_print_all(cell, fp);
       if (cell->tag == CELL_CONS) {
           fputc(')', fp);
       }
    }

    if (eol) {
        fputc('\n', fp);
    }
}

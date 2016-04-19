#include <stdlib.h>
#include "cell.h"

static Cell cell_nil;
Cell* nil = &cell_nil;

void cell_destroy(Cell* cell)
{
    free(cell);
}

Cell* cell_create_int(int value)
{
    Cell* cell = (Cell*) malloc(sizeof(Cell));
    cell->tag = CELL_INT;
    cell->ival = value;
    return cell;
}

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
            fprintf(fp, "%d", cell->ival);
            break;

        case CELL_CONS: {
            const Cons* cons = &cell->cons;
            if (cons->car == nil) {
                fprintf(fp, "nil");
            }
            else if (cons->car->tag == CELL_CONS) {
                fprintf(fp, "(");
                cell_print_all(cons->car, fp);
                fprintf(fp, ")");
            }
            else {
                cell_print_all(cons->car, fp);
            }

            if (cons->cdr == nil) {
            }
            else if (cons->cdr->tag == CELL_CONS) {
                fprintf(fp, " ");
                cell_print_all(cons->cdr, fp);
            }
            else {
                fprintf(fp, " . ");
                cell_print_all(cons->cdr, fp);
            }
            break;
        }
    }
}

void cell_print(const Cell* cell, FILE* fp, int eol)
{
    if (cell == nil) {
        fprintf(fp, "nil");
    }
    else {
       if (cell->tag == CELL_CONS) {
           fprintf(fp, "%c", '(');
       }
       cell_print_all(cell, fp);
       if (cell->tag == CELL_CONS) {
           fprintf(fp, "%c", ')');
       }
    }

    if (eol) {
        fprintf(fp, "\n");
    }
}

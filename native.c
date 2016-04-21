#include <stdio.h>
#include "cell.h"
#include "native.h"

Cell* func_add(Cell* args)
{
    long ret = 0;
    printf("Entering native add\n");
    int pos = 0;
    for (Cell* c = args; c != nil; c = c->cons.cdr, ++pos) {
        Cell* arg = c->cons.car;
        printf("Arg #%d tag %d\n", pos, arg->tag);
        if (arg->tag != CELL_INT) {
            continue;
        }
        ret += arg->ival;
    }
    printf("Leaving native add: %ld\n", ret);
    return cell_create_int(ret);

    return nil;
}

Cell* func_mul(Cell* args)
{
    long ret = 1;
    printf("Entering native mul\n");
    int pos = 0;
    for (Cell* c = args; c != nil; c = c->cons.cdr, ++pos) {
        Cell* arg = c->cons.car;
        printf("Arg #%d tag %d\n", pos, arg->tag);
        if (arg->tag != CELL_INT) {
            continue;
        }
        ret *= arg->ival;
    }
    printf("Leaving native mul: %ld\n", ret);
    return cell_create_int(ret);

    return nil;
}

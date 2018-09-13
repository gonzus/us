#include "us.h"

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    US* us = us_create();
    us_repl(us);
    us_destroy(us);
    return 0;
}

#ifndef US_H_
#define US_H_

// A whole micro-scheme interpreter, including its global
// environment, wrapped in a single embeddable struct.

typedef struct US {
    struct Env* env;
    struct Parser* parser;
} US;

void us_destroy(US* us);
US* us_create(void);

const struct Cell* us_eval_str(US* us, const char* code);

void us_repl(US* us);

#endif

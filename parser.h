#ifndef PARSER_H_
#define PARSER_H_

// Hand-coded parser for a lisp-like language

#include "cell.h"

typedef struct Expression {
    Cell* frst;
    Cell* last;
} Expression;

typedef struct Parser {
    Expression* exp;
    int depth;
    int level;

    int state;
    const char* str;
    int pos;
    int beg;
} Parser;

Parser* parser_create(int depth);
void parser_destroy(Parser* parser);

void parser_parse(Parser* parser, const char* str);
void parser_reset(Parser* parser, const char* str);
Cell* parser_result(Parser* parser);

#endif

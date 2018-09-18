#ifndef PARSER_H_
#define PARSER_H_

// Hand-coded parser for a lisp-like language

#define PARSER_EXP_RESET(e) \
    do { \
        (e).frst = 0; (e).last = 0; \
    } while (0)

#define PARSER_EXP_APPEND(e, c) \
    do { \
        Cell* x = c; \
        if (!((e)->frst)) (e)->frst = x; \
        if ((e)->last) { \
            (e)->last->cons.cdr = x; \
            LOG(INFO, ("APPENDED")); \
        } \
        (e)->last = x; \
    } while (0)

struct US;

typedef struct Expression {
    struct Cell* frst;
    struct Cell* last;
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

void parser_parse(struct US* us, Parser* parser, const char* str);
void parser_reset(Parser* parser, const char* str);
struct Cell* parser_result(Parser* parser);

#endif

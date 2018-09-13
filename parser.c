#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cell.h"
#include "parser.h"

// #define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"

#define PARSER_DEFAULT_DEPTH 128

// Possible parser->states for our parser; it starts in STATE_NORMAL
#define STATE_NORMAL 0
#define STATE_INT    1
#define STATE_REAL   2
#define STATE_STRING 3
#define STATE_SYMBOL 4
#define STATE_LAST   5

// Possible tokens recognized by our parser
#define TOKEN_NONE   0
#define TOKEN_INT    1
#define TOKEN_REAL   2
#define TOKEN_STRING 3
#define TOKEN_SYMBOL 4
#define TOKEN_LPAREN 5
#define TOKEN_RPAREN 6
#define TOKEN_LAST   7

// Check and take action while parsing
#define CHECK_STATE(p, s, t, n, b, c, d) \
    if ((p)->state == (s)) {                 /* check current state  */ \
        if (t != TOKEN_NONE) token(p, t);    /* maybe emit token     */ \
        (p)->state = n;                      /* switch to next state */ \
        if (d >= 0) (p)->beg = (p)->pos + d; /* maybe remember pos   */ \
        if (b) break;                        /* maybe break          */ \
        if (c) continue;                     /* maybe continue       */ \
    } \

static int token(Parser* parser, int token);

Parser* parser_create(int depth)
{
    Parser* parser = (Parser*) calloc(1, sizeof(Parser));
    parser->depth = depth <= 0 ?  PARSER_DEFAULT_DEPTH : depth;
    parser->exp = calloc(parser->depth, sizeof(Expression));

    parser_reset(parser, 0);
    return parser;
}

void parser_destroy(Parser* parser)
{
    free(parser->exp);
    free(parser);
}

void parser_reset(Parser* parser, const char* str)
{
    parser->level = 0;
    parser->exp[0].frst = parser->exp[0].last = 0;
    parser->state = STATE_NORMAL;
    parser->str = str;
    parser->pos = 0;
    parser->beg = 0;
}

Cell* parser_result(Parser* parser)
{
    return parser->exp[0].frst;
}

void parser_parse(Parser* parser, const char* str)
{
    // Our parser is written this way so that it is ready to be translated to a
    // much more efficient table lookup implementation.
    for (parser_reset(parser, str); 1; ++parser->pos) {
        if (str[parser->pos] == '\0') {
            CHECK_STATE(parser, STATE_NORMAL, TOKEN_NONE  , STATE_NORMAL, 1, 0, -1);
            CHECK_STATE(parser, STATE_INT   , TOKEN_INT   , STATE_NORMAL, 1, 0, -1);
            CHECK_STATE(parser, STATE_REAL  , TOKEN_REAL  , STATE_NORMAL, 1, 0, -1);
            CHECK_STATE(parser, STATE_STRING, TOKEN_NONE  , STATE_NORMAL, 1, 0, -1); // ERROR missing closing "
            CHECK_STATE(parser, STATE_SYMBOL, TOKEN_SYMBOL, STATE_NORMAL, 1, 0, -1);
            LOG(FATAL, ("unreachable code -- WTF?"));
        }
        if (str[parser->pos] == '"') {
            CHECK_STATE(parser, STATE_NORMAL, TOKEN_NONE  , STATE_STRING, 0, 1, +1);
            CHECK_STATE(parser, STATE_INT   , TOKEN_INT   , STATE_STRING, 0, 1, +1);
            CHECK_STATE(parser, STATE_REAL  , TOKEN_REAL  , STATE_STRING, 0, 1, +1);
            CHECK_STATE(parser, STATE_SYMBOL, TOKEN_SYMBOL, STATE_STRING, 0, 1, +1);
            CHECK_STATE(parser, STATE_STRING, TOKEN_STRING, STATE_NORMAL, 0, 1, +1);
            LOG(FATAL, ("unreachable code -- WTF?"));
        }
        if (str[parser->pos] == '(') {
            CHECK_STATE(parser, STATE_STRING, TOKEN_NONE  , STATE_STRING, 0, 1, -1);
            CHECK_STATE(parser, STATE_NORMAL, TOKEN_LPAREN, STATE_NORMAL, 0, 1, -1);

            CHECK_STATE(parser, STATE_INT   , TOKEN_INT   , STATE_INT   , 0, 0, -1); // 34( is two tokens
            CHECK_STATE(parser, STATE_INT   , TOKEN_LPAREN, STATE_NORMAL, 0, 1, -1);

            CHECK_STATE(parser, STATE_REAL  , TOKEN_REAL  , STATE_REAL  , 0, 0, -1); // 34.7( is two tokens
            CHECK_STATE(parser, STATE_REAL  , TOKEN_LPAREN, STATE_NORMAL, 0, 1, -1);

            CHECK_STATE(parser, STATE_SYMBOL, TOKEN_SYMBOL, STATE_SYMBOL, 0, 0, -1); // abc( is two tokens
            CHECK_STATE(parser, STATE_SYMBOL, TOKEN_LPAREN, STATE_NORMAL, 0, 1, -1);
            LOG(FATAL, ("unreachable code -- WTF?"));
        }
        if (str[parser->pos] == ')') {
            CHECK_STATE(parser, STATE_STRING, TOKEN_NONE  , STATE_STRING, 0, 1, -1);
            CHECK_STATE(parser, STATE_NORMAL, TOKEN_RPAREN, STATE_NORMAL, 0, 1, -1);

            CHECK_STATE(parser, STATE_INT   , TOKEN_INT   , STATE_INT   , 0, 0, -1); // 34) is two tokens
            CHECK_STATE(parser, STATE_INT   , TOKEN_RPAREN, STATE_NORMAL, 0, 1, -1);

            CHECK_STATE(parser, STATE_REAL  , TOKEN_REAL  , STATE_REAL  , 0, 0, -1); // 34.7) is two tokens
            CHECK_STATE(parser, STATE_REAL  , TOKEN_RPAREN, STATE_NORMAL, 0, 1, -1);

            CHECK_STATE(parser, STATE_SYMBOL, TOKEN_SYMBOL, STATE_SYMBOL, 0, 0, -1); // abc) is two tokens
            CHECK_STATE(parser, STATE_SYMBOL, TOKEN_RPAREN, STATE_NORMAL, 0, 1, -1);
            LOG(FATAL, ("unreachable code -- WTF?"));
        }
        if (str[parser->pos] == '+' ||
            str[parser->pos] == '-') {
            CHECK_STATE(parser, STATE_NORMAL, TOKEN_NONE  , STATE_INT   , 0, 1, +0);
            CHECK_STATE(parser, STATE_INT   , TOKEN_NONE  , STATE_SYMBOL, 0, 1, -1);
            CHECK_STATE(parser, STATE_REAL  , TOKEN_NONE  , STATE_SYMBOL, 0, 1, -1);
            CHECK_STATE(parser, STATE_STRING, TOKEN_NONE  , STATE_STRING, 0, 1, -1);
            CHECK_STATE(parser, STATE_SYMBOL, TOKEN_NONE  , STATE_SYMBOL, 0, 1, -1);
            LOG(FATAL, ("unreachable code -- WTF?"));
        }
        if (str[parser->pos] == '.') {
            CHECK_STATE(parser, STATE_NORMAL, TOKEN_NONE  , STATE_REAL  , 0, 1, +0);
            CHECK_STATE(parser, STATE_INT   , TOKEN_NONE  , STATE_REAL  , 0, 1, -1);
            CHECK_STATE(parser, STATE_REAL  , TOKEN_NONE  , STATE_SYMBOL, 0, 1, -1); // 123.4. is a valid symbol
            CHECK_STATE(parser, STATE_STRING, TOKEN_NONE  , STATE_STRING, 0, 1, -1);
            CHECK_STATE(parser, STATE_SYMBOL, TOKEN_NONE  , STATE_SYMBOL, 0, 1, -1);
            LOG(FATAL, ("unreachable code -- WTF?"));
        }
        if (isspace(str[parser->pos])) {
            CHECK_STATE(parser, STATE_NORMAL, TOKEN_NONE  , STATE_NORMAL, 0, 1, -1);
            CHECK_STATE(parser, STATE_INT   , TOKEN_INT   , STATE_NORMAL, 0, 1, -1);
            CHECK_STATE(parser, STATE_REAL  , TOKEN_REAL  , STATE_NORMAL, 0, 1, -1);
            CHECK_STATE(parser, STATE_SYMBOL, TOKEN_SYMBOL, STATE_NORMAL, 0, 1, -1);
            CHECK_STATE(parser, STATE_STRING, TOKEN_NONE  , STATE_STRING, 0, 1, -1);
            LOG(FATAL, ("unreachable code -- WTF?"));
        }
        if (isdigit(str[parser->pos])) {
            CHECK_STATE(parser, STATE_NORMAL, TOKEN_NONE  , STATE_INT   , 0, 1, +0);
            CHECK_STATE(parser, STATE_INT   , TOKEN_NONE  , STATE_INT   , 0, 1, -1);
            CHECK_STATE(parser, STATE_REAL  , TOKEN_NONE  , STATE_REAL  , 0, 1, -1);
            CHECK_STATE(parser, STATE_STRING, TOKEN_NONE  , STATE_STRING, 0, 1, -1);
            CHECK_STATE(parser, STATE_SYMBOL, TOKEN_NONE  , STATE_SYMBOL, 0, 1, -1);
            LOG(FATAL, ("unreachable code -- WTF?"));
        }
        if (1) {
            CHECK_STATE(parser, STATE_NORMAL, TOKEN_NONE  , STATE_SYMBOL, 0, 1, +0);
            CHECK_STATE(parser, STATE_INT   , TOKEN_NONE  , STATE_SYMBOL, 0, 1, -1); // 1/   is a valid symbol
            CHECK_STATE(parser, STATE_REAL  , TOKEN_NONE  , STATE_SYMBOL, 0, 1, -1); // 1.4/ is a valid symbol
            CHECK_STATE(parser, STATE_STRING, TOKEN_NONE  , STATE_STRING, 0, 1, -1);
            CHECK_STATE(parser, STATE_SYMBOL, TOKEN_NONE  , STATE_SYMBOL, 0, 1, -1);
            LOG(FATAL, ("unreachable code -- WTF?"));
        }
        LOG(FATAL, ("unreachable code -- WTF?"));
    }
}

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
static const char* State[STATE_LAST] = {
    "STATE_NORMAL",
    "STATE_INT",
    "STATE_REAL",
    "STATE_STRING",
    "STATE_SYMBOL",
};
static const char* Token[TOKEN_LAST] = {
    "TOKEN_NONE",
    "TOKEN_INT",
    "TOKEN_REAL",
    "TOKEN_STRING",
    "TOKEN_SYMBOL",
    "TOKEN_LPAREN",
    "TOKEN_RPAREN",
};
#endif

static int token(Parser* parser, int token)
{
    const char* tok = parser->str + parser->beg;;
    int len = 0;
    switch (token) {
        case TOKEN_INT:
        case TOKEN_REAL:
        case TOKEN_STRING:
        case TOKEN_SYMBOL:
            len = parser->pos - parser->beg;
            break;
    }

    // There are some degenerate cases that are recognized
    // as numbers, but should really be symbols.
    if (len == 1 &&
        (token == TOKEN_INT ||
         token == TOKEN_REAL) &&
        (tok[0] == '+' ||
         tok[0] == '-' ||
         tok[0] == '.')) {
        token = TOKEN_SYMBOL;
    }

    if (len > 0) {
        LOG(DEBUG, ("%-15.15s: [%*.*s] (%d)", Token[token], len, len, tok, len));
    }
    else {
        LOG(DEBUG, ("%-15.15s", Token[token]));
    }

    const Cell* cell = 0;
    switch (token) {
        case TOKEN_INT:
            cell = cell_create_int_from_string(tok, len);
            break;

        case TOKEN_REAL:
            cell = cell_create_real_from_string(tok, len);
            break;

        case TOKEN_STRING:
            cell = cell_create_string(tok, len);
            break;

        case TOKEN_SYMBOL:
            if (memcmp(tok, CELL_STR_BOOL_T, sizeof(CELL_STR_BOOL_T) - 1) == 0) {
                cell = bool_t; // Special case: #t
                break;
            }
            if (memcmp(tok, CELL_STR_BOOL_F, sizeof(CELL_STR_BOOL_F) - 1) == 0) {
                cell = bool_f; // Special case: #f
                break;
            }
            cell = cell_create_symbol(tok, len);
            break;

        case TOKEN_LPAREN:
            ++parser->level;
            parser->exp[parser->level].frst = 0;
            parser->exp[parser->level].last = 0;
            break;

        case TOKEN_RPAREN:
            cell = parser->exp[parser->level].frst;
            --parser->level;
            if (!cell) {
                cell = nil; // Special case: () => nil
            }
            break;

        default:
            break;
    }
    if (!cell) {
        return 0;
    }

    Expression* exp = &parser->exp[parser->level];
    if (parser->level == 0) {
        exp->frst = (Cell*) cell;
        exp->last = 0;
        return 0;
    }

    Cell* cons = cell_cons(cell, nil);
    if (!exp->frst) {
        exp->frst = cons;
    }
    if (exp->last) {
        exp->last->cons.cdr = cons;
    }
    exp->last = cons;

    return 0;
}

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "cell.h"
#include "parser.h"

#define PARSER_DEFAULT_DEPTH 128

// Possible parser->states for our parser; it starts in STATE_NORMAL
#define STATE_NORMAL 0
#define STATE_NUMBER 1
#define STATE_STRING 2
#define STATE_SYMBOL 3
#define STATE_LAST   4

// Possible tokens recognized by our parser
#define TOKEN_NONE   0
#define TOKEN_NUMBER 1
#define TOKEN_STRING 2
#define TOKEN_SYMBOL 3
#define TOKEN_LPAREN 4
#define TOKEN_RPAREN 5
#define TOKEN_LAST   6

static int token(Parser* parser, int token);

Parser* parser_create(int depth)
{
    Parser* parser = (Parser*) malloc(sizeof(Parser));
    parser->depth = depth <= 0 ?  PARSER_DEFAULT_DEPTH : depth;
    parser->exp = calloc(parser->depth, sizeof(Expression));
    parser->level = 0;

    parser->state = STATE_NORMAL;
    parser->str = 0;
    parser->pos = 0;
    parser->beg = 0;
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
    parser->state = STATE_NORMAL;
    parser->str = str;
    parser->pos = 0;
    parser->beg = 0;
}

void parser_parse(Parser* parser, const char* str)
{
    parser_reset(parser, str);
    for (; ; ++parser->pos) {
        if (str[parser->pos] == '\0') {
            if (parser->state == STATE_NORMAL) {
            }
            else if (parser->state == STATE_NUMBER) {
                token(parser, TOKEN_NUMBER);
                parser->state = STATE_NORMAL;
            }
            else if (parser->state == STATE_STRING) {
                // ERROR missing closing "
                parser->state = STATE_NORMAL;
            }
            else if (parser->state == STATE_SYMBOL) {
                token(parser, TOKEN_SYMBOL);
                parser->state = STATE_NORMAL;
            }
            break; // QUIT LOOP
        }
        else if (str[parser->pos] == '"') {
            if (parser->state == STATE_NORMAL) {
                parser->state = STATE_STRING;
            }
            else if (parser->state == STATE_NUMBER) {
                token(parser, TOKEN_NUMBER);
                parser->state = STATE_STRING;
            }
            else if (parser->state == STATE_STRING) {
                token(parser, TOKEN_STRING);
                parser->state = STATE_NORMAL;
            }
            else if (parser->state == STATE_SYMBOL) {
                token(parser, TOKEN_SYMBOL);
                parser->state = STATE_STRING;
            }
            parser->beg = parser->pos + 1;
        }
        else if (str[parser->pos] == '(') {
            if (parser->state == STATE_NORMAL) {
                token(parser, TOKEN_LPAREN);
            }
            else if (parser->state == STATE_NUMBER) {
                // TODO: 34( is a valid symbol? For now, not.
                token(parser, TOKEN_NUMBER);
                token(parser, TOKEN_LPAREN);
                parser->state = STATE_NORMAL;
            }
            else if (parser->state == STATE_STRING) {
            }
            else if (parser->state == STATE_SYMBOL) {
                // TODO: ab( is a valid symbol? For now, not.
                token(parser, TOKEN_SYMBOL);
                token(parser, TOKEN_LPAREN);
                parser->state = STATE_NORMAL;
            }
        }
        else if (str[parser->pos] == ')') {
            if (parser->state == STATE_NORMAL) {
                token(parser, TOKEN_RPAREN);
            }
            else if (parser->state == STATE_NUMBER) {
                // TODO: 34) is a valid symbol? For now, not.
                token(parser, TOKEN_NUMBER);
                token(parser, TOKEN_RPAREN);
                parser->state = STATE_NORMAL;
            }
            else if (parser->state == STATE_STRING) {
            }
            else if (parser->state == STATE_SYMBOL) {
                // TODO: ab) is a valid symbol? For now, not.
                token(parser, TOKEN_SYMBOL);
                token(parser, TOKEN_RPAREN);
                parser->state = STATE_NORMAL;
            }
        }
        else if (isspace(str[parser->pos])) {
            if (parser->state == STATE_NORMAL) {
            }
            else if (parser->state == STATE_NUMBER) {
                token(parser, TOKEN_NUMBER);
                parser->state = STATE_NORMAL;
            }
            else if (parser->state == STATE_STRING) {
            }
            else if (parser->state == STATE_SYMBOL) {
                token(parser, TOKEN_SYMBOL);
                parser->state = STATE_NORMAL;
            }
        }
        else if (isdigit(str[parser->pos])) {
            if (parser->state == STATE_NORMAL) { parser->beg = parser->pos;
                parser->state = STATE_NUMBER;
            }
            else if (parser->state == STATE_NUMBER) {
            }
            else if (parser->state == STATE_STRING) {
            }
            else if (parser->state == STATE_SYMBOL) {
            }
        }
        else {
            if (parser->state == STATE_NORMAL) {
                parser->beg = parser->pos;
                parser->state = STATE_SYMBOL;
            }
            else if (parser->state == STATE_NUMBER) {
                // 1+ is a valid symbol, simply change parser->states, leave parser->beg alone
                parser->state = STATE_SYMBOL;
            }
            else if (parser->state == STATE_STRING) {
            }
            else if (parser->state == STATE_SYMBOL) {
            }
        }
    }
}

static int token(Parser* parser, int token)
{
    static const char* Token[TOKEN_LAST] = {
        "TOKEN_NONE",
        "TOKEN_NUMBER",
        "TOKEN_STRING",
        "TOKEN_SYMBOL",
        "TOKEN_LPAREN",
        "TOKEN_RPAREN",
    };

    const char* tok = parser->str + parser->beg;;
    printf("%-15.15s", Token[token]);
    int len = 0;
    switch (token) {
        case TOKEN_NUMBER:
        case TOKEN_STRING:
        case TOKEN_SYMBOL:
            len = parser->pos - parser->beg;
            printf(": [%*.*s]", len, len, tok);
            break;
    }
    printf("\n");
    fflush(stdout);

    Cell* cell = 0;
    switch (token) {
        case TOKEN_NUMBER:
            cell = cell_create_int_from_string(tok, len);
            break;

        case TOKEN_STRING:
            cell = cell_create_string(tok, len);
            break;

        case TOKEN_SYMBOL:
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
            break;

        case TOKEN_NONE:
        default:
            break;
    }
    if (!cell) {
        return 0;
    }

    Expression* exp = &parser->exp[parser->level];
    if (parser->level == 0) {
        exp->frst = cell;
        exp->last = 0;
        return 0;
    }

    cell = cell_cons(cell, nil);
    if (!exp->frst) {
        exp->frst = cell;
    }
    if (!exp->last) {
        exp->last = cell;
    }
    else {
        exp->last->cons.cdr = cell;
        exp->last = cell;
    }

    return 0;
}

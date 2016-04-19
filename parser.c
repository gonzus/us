#include <ctype.h>
#include <stdio.h>
#include "cell.h"
#include "parser.h"

// Possible states for our parser; it starts in STATE_NORMAL
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

int parse(const char* str)
{
    int state = STATE_NORMAL;
    int beg = -1;  // beginning position for number, string, symbol
    int pos = 0;   // current position
    while (1) {
        if (str[pos] == '\0') {
            if (state == STATE_NORMAL) {
            }
            else if (state == STATE_NUMBER) {
                token(str, TOKEN_NUMBER, beg, pos);
                beg = -1;
                state = STATE_NORMAL;
            }
            else if (state == STATE_STRING) {
                // ERROR missing closing "
                beg = -1;
                state = STATE_NORMAL;
            }
            else if (state == STATE_SYMBOL) {
                token(str, TOKEN_SYMBOL, beg, pos);
                beg = -1;
                state = STATE_NORMAL;
            }
            break; // QUIT LOOP
        }
        else if (str[pos] == '"') {
            if (state == STATE_NORMAL) {
                state = STATE_STRING;
                beg = pos + 1;
            }
            else if (state == STATE_NUMBER) {
                token(str, TOKEN_NUMBER, beg, pos);
                beg = pos + 1;
                state = STATE_STRING;
            }
            else if (state == STATE_STRING) {
                token(str, TOKEN_STRING, beg, pos);
                beg = -1;
                state = STATE_NORMAL;
            }
            else if (state == STATE_SYMBOL) {
                token(str, TOKEN_SYMBOL, beg, pos);
                beg = pos + 1;
                state = STATE_STRING;
            }
        }
        else if (str[pos] == '(') {
            if (state == STATE_NORMAL) {
                token(str, TOKEN_LPAREN, pos, pos + 1);
            }
            else if (state == STATE_NUMBER) {
                token(str, TOKEN_NUMBER, beg, pos);
                beg = -1;
                token(str, TOKEN_LPAREN, pos, pos + 1);
                state = STATE_NORMAL;
            }
            else if (state == STATE_STRING) {
            }
            else if (state == STATE_SYMBOL) {
                token(str, TOKEN_SYMBOL, beg, pos);
                beg = -1;
                token(str, TOKEN_LPAREN, pos, pos + 1);
                state = STATE_NORMAL;
            }
        }
        else if (str[pos] == ')') {
            if (state == STATE_NORMAL) {
                token(str, TOKEN_RPAREN, pos, pos + 1);
            }
            else if (state == STATE_NUMBER) {
                token(str, TOKEN_NUMBER, beg, pos);
                beg = -1;
                token(str, TOKEN_RPAREN, pos, pos + 1);
                state = STATE_NORMAL;
            }
            else if (state == STATE_STRING) {
            }
            else if (state == STATE_SYMBOL) {
                token(str, TOKEN_SYMBOL, beg, pos);
                beg = -1;
                token(str, TOKEN_RPAREN, pos, pos + 1);
                state = STATE_NORMAL;
            }
        }
        else if (isspace(str[pos])) {
            if (state == STATE_NORMAL) {
            }
            else if (state == STATE_NUMBER) {
                token(str, TOKEN_NUMBER, beg, pos);
                beg = -1;
                state = STATE_NORMAL;
            }
            else if (state == STATE_STRING) {
            }
            else if (state == STATE_SYMBOL) {
                token(str, TOKEN_SYMBOL, beg, pos);
                beg = -1;
                state = STATE_NORMAL;
            }
        }
        else if (isdigit(str[pos])) {
            if (state == STATE_NORMAL) {
                state = STATE_NUMBER;
                beg = pos;
            }
            else if (state == STATE_NUMBER) {
            }
            else if (state == STATE_STRING) {
            }
            else if (state == STATE_SYMBOL) {
            }
        }
        else {
            if (state == STATE_NORMAL) {
                beg = pos;
                state = STATE_SYMBOL;
            }
            else if (state == STATE_NUMBER) {
                token(str, TOKEN_NUMBER, beg, pos);
                beg = pos + 1;
                state = STATE_SYMBOL;
            }
            else if (state == STATE_STRING) {
            }
            else if (state == STATE_SYMBOL) {
            }
        }

        ++pos;
    }

    return state;
}

typedef struct Level {
    Cell* frst;
    Cell* last;
} Level;

typedef struct Parser {
    Level level[128];
    int pos;
} Parser;

static Parser parser;

int token(const char* str, int token, int beg, int end)
{
    static const char* Token[TOKEN_LAST] = {
        "TOKEN_NONE",
        "TOKEN_NUMBER",
        "TOKEN_STRING",
        "TOKEN_SYMBOL",
        "TOKEN_LPAREN",
        "TOKEN_RPAREN",
    };
    const char* tok = str + beg;;
    int len = end - beg;
    printf("%-15.15s: [%*.*s]\n", Token[token], len, len, tok);
    fflush(stdout);

    Level* level = &parser.level[parser.pos];
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
            ++parser.pos;
            level->frst = 0;
            level->last = 0;
            break;

        case TOKEN_RPAREN:
            level->frst = 0;
            level->last = 0;
            --parser.pos;
            break;

        case TOKEN_NONE:
        default:
            break;
    }
    if (cell) {
        cell = cell_cons(cell, nil);
        if (!level->frst) {
            level->frst = cell;
        }
        if (!level->last) {
            level->last = cell;
        }
        else {
            level->last->cons.cdr = cell;
            level->last = cell;
        }
    }

    return 0;
}

#include <ctype.h>
#include <stdio.h>
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

// For now we just print the token we recognized
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
    int len = end - beg;
    fprintf(stderr, "%-15.15s: [%*.*s]\n", Token[token], len, len, str + beg);
    return 0;
}

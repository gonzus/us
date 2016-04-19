#ifndef PARSER_H_
#define PARSER_H_

// Hand-coded parser for a lisp-like language

int parse(const char* str);
int token(const char* str, int token, int beg, int end);

#endif

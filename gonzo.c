#include <stdio.h>
#include <string.h>
#include "cell.h"
#include "parser.h"
#include "env.h"
#include "us.h"

static int test_cell(const char* label, const Cell* cell, const char* expected)
{
    char dumper[10*1024];
    cell_dump(cell, 0, dumper);
    int ok = strcmp(dumper, expected) == 0;
    if (ok) {
        printf("ok %s got [%s]\n", label, dumper);
    }
    else {
        printf("BAD %s got [%s], expected [%s]\n", label, dumper, expected);
    }
    return ok;
}

static void test_globals(void)
{
    test_cell("cell", nil   , "()");
    test_cell("cell", bool_t, "#t");
    test_cell("cell", bool_f, "#f");
}

static void test_strings(void)
{
    static struct {
        const char* value;
    } data[] = {
        { "" },
        { "bilbo" },
        { "bilbo & frodo" },
        { "In a hole in the ground there lived a hobbit..." },
        { "Khazad-dûm" },
    };
    int n = sizeof(data) / sizeof(data[0]);
    for (int j = 0; j < n; ++j) {
        const char* value = data[j].value;
        char expected[1024];
        sprintf(expected, "\"%s\"", value);
        test_cell("string", cell_create_string(value, 0), expected);
    }
}

static void test_integers(void)
{
    static struct {
        long value;
    } data[] = {
        { 0 },
        { 1 },
        { -1 },
        { 1984 },
        { 2001 },
        { -123456 },
    };

    int n = sizeof(data) / sizeof(data[0]);
    for (int j = 0; j < n; ++j) {
        long value = data[j].value;
        char expected[1024];
        sprintf(expected, "%ld", value);
        test_cell("int", cell_create_int(value), expected);
    }
}

static void test_reals(void)
{
    static struct {
        double value;
    } data[] = {
        {  0.0 },
        {  0.1 },
        { -0.1 },
        {  1.0 },
        { -1.0 },
        { 22.0 / 7.0 },
        { 355.0 / 113.0 },
    };

    int n = sizeof(data) / sizeof(data[0]);
    for (int j = 0; j < n; ++j) {
        double value = data[j].value;
        char expected[1024];
        sprintf(expected, "%lf", value);
        test_cell("real", cell_create_real(value), expected);
    }
}

static void test_simple_list(void)
{
    const Cell* c = cell_cons(cell_create_int(1),
                        cell_cons(cell_create_int(2),
                                  cell_cons(cell_create_int(3), nil)));
    test_cell("simple_list", c, "(1 2 3)");
}

static void test_nested_list(void)
{
    const Cell* c23 = cell_cons(cell_create_int(2),
                          cell_cons(cell_create_int(3), nil));
    const Cell* c = cell_cons(cell_create_int(1),
                        cell_cons(c23,
                                  cell_cons(cell_create_int(4), nil)));
    test_cell("nested_list", c, "(1 (2 3) 4)");
}

static void test_dotted_list(void)
{
    const Cell* c = cell_cons(cell_create_int(1),
                        cell_create_int(2));
    test_cell("dotted_list", c, "(1 . 2)");
}

static void test_lists(void)
{
    test_simple_list();
    test_nested_list();
    test_dotted_list();
}

static void test_symbol(void)
{
    do {
        Env* parent = env_create(0);
        if (parent) {
            printf("ok symbol created parent env\n");
        }
        else {
            printf("BAD symbol created parent env\n");
            break;
        }

        const char* name = "set-gonzo!";

        Symbol* s1 = env_lookup(parent, name, 1);
        if (s1) {
            printf("ok symbol created sym [%s]\n", name);
        }
        else {
            printf("BAD symbol created sym [%s]\n", name);
            break;
        }

        const long value = 11;
        const Cell* c = cell_create_int(value);
        if (c) {
            printf("ok symbol created cell [%ld]\n", c->ival);
        }
        else {
            printf("BAD symbol created cell [%ld] [%ld]\n", c->ival, value);
            break;
        }

        s1->value = c;

        Symbol* s2 = env_lookup(parent, name, 0);
        if (s2) {
            printf("ok symbol fetched sym [%s]\n", name);
        }
        else {
            printf("BAD symbol fetched sym [%s]\n", name);
            break;
        }

        Env* child = env_create(0);
        if (child) {
            printf("ok symbol created child env\n");
        }
        else {
            printf("BAD symbol created child env\n");
            break;
        }

        env_chain(child, parent);

        Symbol* s3 = env_lookup(child, name, 0);
        if (s3) {
            printf("ok symbol fetched sym [%s]\n", name);
        }
        else {
            printf("BAD symbol fetched sym [%s]\n", name);
            break;
        }

        if (s3->value == c) {
            printf("ok symbol values match [%ld]\n", s3->value->ival);
        }
        else {
            printf("BAD symbol values match [%ld] [%ld]\n", s3->value->ival, value);
            break;
        }
    } while (0);
}

static void test_parser(void)
{
    static struct {
        const char* code;
        const char* expected;
    } data[] = {
        { " () ", "()" },
        { " 11 ", "11" },
        { " -3.1415 ", "-3.141500" },
        {
            " ( 1\"hi\"2 3.3\"ho\"4.4 (2 3)\"hu\"(6 7) ) ",
            "(1 \"hi\" 2 3.300000 \"ho\" 4.400000 (2 3) \"hu\" (6 7))"
        },
        {
            " ( +1 -2 3. 4. +5.5 -6.6 +.7 .8 -.9 #t #f () a b c ) ",
            "(1 -2 3.000000 4.000000 5.500000 -6.600000 0.700000 0.800000 -0.900000 #t #f () a b c)"
        },
        {
            " ( + - * / . % ! @ # $ ^ & ) ",
            "(+ - * / . % ! @ # $ ^ &)"
        },
        {
            " \"The Hobbit rules!\" ",
            "\"The Hobbit rules!\""
        },
        {
            " (1 2 3) ",
            "(1 2 3)"
        },
        {
            " (1 4+ 5- 6.1* 7.4/ 1.2.3 2 3 4.655 3[6]) ",
            "(1 4+ 5- 6.1* 7.4/ 1.2.3 2 3 4.655000 3[6])"
        },
        {
            " (1 2 (a b c) 3 (4 x (5 y))) ",
            "(1 2 (a b c) 3 (4 x (5 y)))"
        },
        {
            "  ( define    fact   (lambda x (if(< x 2) 1 (* x(fact(1- x))))))  ",
            "(define fact (lambda x (if (< x 2) 1 (* x (fact (1- x))))))"
        },
        {
            "(define       (sum n) (if (zero? n  )         0 (+ n (sum (sub1 n)))))",
            "(define (sum n) (if (zero? n) 0 (+ n (sum (sub1 n)))))"
        },
    };

    Parser* parser = parser_create(0);
    int n = sizeof(data) / sizeof(data[0]);
    for (int j = 0; j < n; ++j) {
        const char* code = data[j].code;
        const char* expected = data[j].expected;
        parser_parse(parser, code);
        test_cell("parse", parser_result(parser), expected);
    }
    parser_destroy(parser);
}

static void test_eval_simple(void)
{
    static struct {
        const char* expected;
        const char* code;
    } data[] = {
        { "11", " 11 " },
        { "-3.141500", " -3.1415 " },
        { "7", " (+ 3 4) " },
        { "10", " (+ 1 2 3 4) " },
        { "15", " (+ 3 (+ 4 5) (+ 1 2)) " },
        { "-9", " (- 9) " },
        { "5", " (- 9 4) " },
        { "3", " (- 9 4 2) " },
        { "-9.000000", " (- 9.0) " },
        { "5.000000", " (- 9.0 4.0) " },
        { "3.000000", " (- 9.0 4.0 2.0) " },
        { "5.000000", " (- 9 4.0) " },
        { "3.000000", " (- 9 4.0 2) " },
        { "3.000000", " (- 9 4.0 2.0) " },
        { "3.000000", " (- 9 4 2.0) " },
        { "6", " (* 2 3) " },
        { "24", " (* 1 2 3 4) " },
        { "720", " (* 2 (* 3 4) (* 5 6)) " },
        { "736", " (* 2 (+ 3 (* 5 4)) (+ (* 5 2) 6)) " },
        { "7.500000", " (+ 3 4.5) " },
        { "8.000000", " (+ 3.5 4.5) " },
        { "()", " (+) " },
        { "()", " (-) " },
        { "13.500000", " (* 3 4.5) " },
        { "15.750000", " (* 3.5 4.5) " },
        { "()", " (*) " },
        { "()", " (/) " },
        { "()", " (/ 0) " },
        { "()", " (/ 0.0) " },
        { "()", " (/ 3 0) " },
        { "()", " (/ 3 0.0) " },
        { "()", " (/ 3.0 0) " },
        { "()", " (/ 3.0 0.0) " },
        { "1", " (/ 1) " },
        { "-1", " (/ -1) " },
        { "1.000000", " (/ 1.0) " },
        { "-1.000000", " (/ -1.0) " },
        { "0.500000", " (/ 2) " },
        { "-0.500000", " (/ -2) " },
        { "0.500000", " (/ 2.0) " },
        { "-0.500000", " (/ -2.0) " },
        { "6", " (/ 24 4) " },
        { "4.800000", " (/ 24 5) " },
        { "6.000000", " (/ 24.0 4) " },
        { "4.800000", " (/ 24.0 5) " },
        { "6.000000", " (/ 24 4.0) " },
        { "4.800000", " (/ 24 5.0) " },
        { "6.000000", " (/ 24.0 4.0) " },
        { "4.800000", " (/ 24.0 5.0) " },
        { "2", " (/ 24 4 3) " },
        { "1.200000", " (/ 24 4 5) " },
        { "2.000000", " (/ 24.0 4 3) " },
        { "1.200000", " (/ 24.0 4 5) " },
        { "2.000000", " (/ 24 4.0 3) " },
        { "1.200000", " (/ 24 4.0 5) " },
        { "2.000000", " (/ 24.0 4.0 3) " },
        { "1.200000", " (/ 24.0 4.0 5) " },
        { "2.000000", " (/ 24 4 3.0) " },
        { "1.200000", " (/ 24 4 5.0) " },
        { "2.000000", " (/ 24.0 4 3.0) " },
        { "1.200000", " (/ 24.0 4 5.0) " },
        { "2.000000", " (/ 24 4.0 3.0) " },
        { "1.200000", " (/ 24 4.0 5.0) " },
        { "2.000000", " (/ 24.0 4.0 3.0) " },
        { "1.200000", " (/ 24.0 4.0 5.0) " },

        { "()", " () " },
        { "#t", " #t " },
        { "#f", " #f " },
        { "(+ 3 4)", " (quote (+ 3 4)) " },
        { "11", " (define gonzus 11) " },
        { "11", " gonzus " },
        { "77", " (set! gonzus (* 7 11)) " },
        { "77", " gonzus " },
        { "()", " (set! invalid (+ 2 3)) " },  // this generates an error msg
        { "()", " invalid " },
        { "#t", " (=) " },
        { "#t", " (= 7) " },
        { "#f", " (= 7 11) " },
        { "#t", " (= 11 11) " },
        { "#f", " (= 7 11.0) " },
        { "#f", " (= 7.0 11) " },
        { "#f", " (= 7. 11.0) " },
        { "#t", " (= 7.0 7.0) " },
        { "#f", " (= 7 11 8) " },
        { "#t", " (= 7 7 7) " },
        { "#t", " (= \"Bilbo\" \"Bilbo\") " },
        { "#f", " (= \"Bilbo\" \"Frodo\") " },
        { "#t", " (= + +) " },
        { "#f", " (= + *) " },
        { "\"Sane\"", " (if (= 7 11) \"Crazy!\" \"Sane\")  " },
        { "7", " (if (= \"abc\" \"abc\") (+ 3 4) (- 5 6)) " },
        { "\"Positive!\"", " (if (> 13 0)  \"Positive!\" \"Negative\")  " },
        { "\"Negative\"", " (if (> 13 20) \"Positive!\" \"Negative\")  " },
        { "\"Negative\"", " (if (< 13 0)  \"Positive!\" \"Negative\")  " },
        { "\"Positive!\"", " (if (< 13 20) \"Positive!\" \"Negative\")  " },
    };

    US* us = us_create();
    int n = sizeof(data) / sizeof(data[0]);
    for (int j = 0; j < n; ++j) {
        const char* code = data[j].code;
        const char* expected = data[j].expected;
        test_cell("eval_simple", us_eval_str(us, code), expected);
    }
    us_destroy(us);
}

static void test_eval_complex(void)
{
    static struct {
        const char* expected;
        const char* code;
    } data[] = {
        { "()", "(define nil (quote ()))" },
        { "<*CODE*>", "(define null? (lambda (L) (= L nil)))" },
        { "<*CODE*>", "(define length (lambda (L) (if (= L nil) 0 (+ 1 (length (cdr L))))))" },
        { "<*CODE*>", "(define empty? (lambda (L) (= 0 (length L))))" },
        { "(1 2 3 4)", "(define L (quote (1 2 3 4)))" },
        { "4", "(length L)" },
        { "#f", "(empty? L)" },

        { "<*CODE*>", " (define positive (lambda (x) (> x 0))) " },
        { "#t", " (positive  7) " },
        { "#f", " (positive -7) " },

        { "<car>", "(define first car)" },
        { "<cdr>", "(define rest cdr)" },
        { "()", "(define nil (quote ()))" },
        { "<*CODE*>", "(define null? (lambda (L) (= L nil)))" },
        { "<*CODE*>", "(define length (lambda (L) (if (= L nil) 0 (+ 1 (length (cdr L))))))" },
        { "<*CODE*>", "(define sum (lambda (L) (if (= L nil) 0 (+ (car L) (sum (cdr L))))))" },
        { "<*CODE*>", "(define count (lambda (item L) (if (null? L) 0 (+ (if (= item (first L)) 1 0) (count item (rest L))))))" },
        { "1", "(count 7 (quote (6 7 8)))" },
        { "3", "(count 0 (quote (0 1 2 3 0 0)))" },
        { "4", "(count (quote the) (quote (the more the merrier the bigger the better)))" },

        { "<*CODE*>", "(define twice (lambda (x) (* 2 x)))" },
        { "20", "(twice 10)" },
        { "<*CODE*>", "(define repeat (lambda (f) (lambda (x) (f (f x)))))" },
        { "40", "((repeat twice) 10)" },
        { "160", "((repeat (repeat twice)) 10)" },
        { "2560", "((repeat (repeat (repeat twice))) 10)" },
        { "655360", "((repeat (repeat (repeat (repeat twice)))) 10)" },

        { "(1 2 3 4)", "(define L (quote (1 2 3 4)))" },
        { "4", "(length L)" },
        { "#f", "(null? L)" },
        { "3", "(sum (quote (1 2)))" },
        { "6", "(sum (quote (1 2 3)))" },
        { "22", "(sum (quote (4 5 6 7)))" },

        { "<*CODE*>", " (define fact (lambda (n) (if (< n 2) 1 (* n (fact (- n 1)))))) " },
        { "120", " (fact 5) " },
        { "3628800", " (fact 10) " },
        { "2432902008176640000", " (fact 20) " },

        { "<*CODE*>", " (define fib (lambda (n) (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2)))))) " },
        { "5", " (fib 5) " },
        { "13", " (fib 7) " },
        { "6765", " (fib 20) " },
        { "<*CODE*>", "(define range (lambda (a b) (if (= a b) (quote ()) (cons a (range (+ a 1) b)))))" },
        { "(0 1 2 3 4 5 6 7 8 9)", "(range 0 10)" },
        { "<*CODE*>", "(define map (lambda (f l) (if (null? l) nil (cons (f (car l)) (map f (cdr l))))))" },
        { "(0 1 1 2 3 5 8 13 21 34)", "(map fib (range 0 10))" },
        { "(0 1 1 2 3 5 8 13 21 34 55 89 144 233 377 610 987 1597 2584 4181)", "(map fib (range 0 20))" },

        { "bofur", " (car (quote (bofur bombur))) " },
        { "(bombur)", " (cdr (quote (bofur bombur))) " },
        { "(bifur bofur bombur)"," (cons (quote bifur) (quote (bofur bombur))) " },

        { "<*CODE*>", " (define make-account (lambda (balance) (lambda (amt) (begin (set! balance (+ balance amt)) balance)))) " },
        { "<*CODE*>", " (define acct1 (make-account 100.0)) " }, // yes, make-account returns lambda
        { "80.000000", " (acct1 -20.0) " },
        { "<*CODE*>", " (define acct2 (make-account 200.0)) " },
        { "180.000000", " (acct2 -20.0) " },
        { "70.000000", " (acct1 -10.0) " },
        { "160.000000", " (acct2 -20.0) " },
    };

    US* us = us_create();
    int n = sizeof(data) / sizeof(data[0]);
    for (int j = 0; j < n; ++j) {
        const char* code = data[j].code;
        const char* expected = data[j].expected;
        test_cell("eval_complex", us_eval_str(us, code), expected);
    }
    us_destroy(us);
}

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    test_globals();
    test_strings();
    test_integers();
    test_reals();
    test_lists();
    test_symbol();
    test_parser();
    test_eval_simple();
    test_eval_complex();

    return 0;
}

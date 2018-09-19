#include <stdio.h>
#include <string.h>
#include "arena.h"
#include "cell.h"
#include "parser.h"
#include "env.h"
#include "us.h"

// #define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"

static void test_arena(void)
{
    static struct {
        int bits[5];
    } data[] = {
        { { -1, -1, -1, -1, -1 } },
        { {  0, -1, -1, -1, -1 } },
        { {  1, -1, -1, -1, -1 } },
        { {  2, -1, -1, -1, -1 } },
        { {  3, -1, -1, -1, -1 } },
        { {  4, -1, -1, -1, -1 } },
        { {  5, -1, -1, -1, -1 } },
        { {  6, -1, -1, -1, -1 } },
        { {  7, -1, -1, -1, -1 } },
        { {  8, -1, -1, -1, -1 } },
        { {  9, -1, -1, -1, -1 } },
        { { 10, -1, -1, -1, -1 } },
        { { 11, -1, -1, -1, -1 } },
        { { 12, -1, -1, -1, -1 } },
        { { 13, -1, -1, -1, -1 } },
        { { 14, -1, -1, -1, -1 } },
        { { 15, -1, -1, -1, -1 } },
        { { 16, -1, -1, -1, -1 } },
        { { 17, -1, -1, -1, -1 } },
        { { 18, -1, -1, -1, -1 } },
        { { 19, -1, -1, -1, -1 } },
        { { 20, -1, -1, -1, -1 } },
        { { 21, -1, -1, -1, -1 } },
        { { 22, -1, -1, -1, -1 } },
        { { 23, -1, -1, -1, -1 } },
        { { 24, -1, -1, -1, -1 } },
        { { 25, -1, -1, -1, -1 } },
        { { 26, -1, -1, -1, -1 } },
        { { 27, -1, -1, -1, -1 } },
        { { 28, -1, -1, -1, -1 } },
        { { 29, -1, -1, -1, -1 } },
        { { 30, -1, -1, -1, -1 } },
        { { 31, -1, -1, -1, -1 } },
        { { 32, -1, -1, -1, -1 } },
        { { 33, -1, -1, -1, -1 } },
        { { 34, -1, -1, -1, -1 } },
        { { 35, -1, -1, -1, -1 } },
        { { 36, -1, -1, -1, -1 } },
        { { 37, -1, -1, -1, -1 } },
        { { 38, -1, -1, -1, -1 } },
        { { 39, -1, -1, -1, -1 } },
        { { 40, -1, -1, -1, -1 } },
        { { 41, -1, -1, -1, -1 } },
        { { 42, -1, -1, -1, -1 } },
        { { 43, -1, -1, -1, -1 } },
        { { 44, -1, -1, -1, -1 } },
        { { 45, -1, -1, -1, -1 } },
        { { 46, -1, -1, -1, -1 } },
        { { 47, -1, -1, -1, -1 } },
        { { 48, -1, -1, -1, -1 } },
        { { 49, -1, -1, -1, -1 } },
        { { 50, -1, -1, -1, -1 } },
        { { 51, -1, -1, -1, -1 } },
        { { 52, -1, -1, -1, -1 } },
        { { 53, -1, -1, -1, -1 } },
        { { 54, -1, -1, -1, -1 } },
        { { 55, -1, -1, -1, -1 } },
        { { 56, -1, -1, -1, -1 } },
        { { 57, -1, -1, -1, -1 } },
        { { 58, -1, -1, -1, -1 } },
        { { 59, -1, -1, -1, -1 } },
        { { 60, -1, -1, -1, -1 } },
        { { 61, -1, -1, -1, -1 } },
        { { 62, -1, -1, -1, -1 } },
        { { 63, -1, -1, -1, -1 } },
        { {  1,  6, -1, -1, -1 } },
        { { 10, 20, 30, 40, 50 } },
        { { 11, 22, 33, 44, 55 } },
        { { 21, 22, 23, 24, 25 } },
        { {  4,  8, 12, 16, 20 } },
        { {  5, 10, 15, -1, -1 } },
        { { 60, 59, 58, 57, 56 } },
        { { 63, 53, 43, 33, 23 } },
    };
    Arena* arena = arena_create();
    Cell* f[64];
    for (int j = 0; j < 64; ++j) {
        f[j] = arena_get_cell(arena, 0);
    }
    int n = sizeof(data) / sizeof(data[0]);
    for (int j = 0; j < n; ++j) {
        uint64_t comp = POOL_EMPTY;
        arena_reset_to_empty(arena);
        for (int k = 0; k < 5; ++k) {
            int b = data[j].bits[k];
            if (b < 0) {
                continue;
            }
            arena_mark_cell_used(arena, f[b]);
            POOL_MARK_USED(comp, b);
        }
        CellPool* p = arena_get_pool_for_cell(arena, f[0]);
        uint64_t last = p->mask;
        if (comp == last) {
            printf("ok %" POOL_MASK_FMT "\n", comp);
        } else {
            printf("BAD %" POOL_MASK_FMT " -- %" POOL_MASK_FMT "\n", comp, last);
        }
    }
    arena_destroy(arena);
}

static int test_cell(const char* label, const Cell* cell, const char* expected)
{
    char dumper[10*1024];
    cell_dump(cell, 0, dumper);
    int ok = strcmp(dumper, expected) == 0;
    if (ok) {
        printf("ok %s got [%s]\n", label, dumper);
    } else {
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

static void test_strings(US* us)
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
        Cell* c = cell_create_string(us, value, 0);
        test_cell("string", c, expected);
    }
}

static void test_integers(US* us)
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
        Cell* c = cell_create_int(us, value);
        test_cell("int", c, expected);
    }
}

static void test_reals(US* us)
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
        Cell* c = cell_create_real(us, value);
        test_cell("real", c, expected);
    }
}

static void test_simple_list(US* us)
{
    Cell* c = cell_cons(us, cell_create_int(us, 1),
                  cell_cons(us, cell_create_int(us, 2),
                            cell_cons(us, cell_create_int(us, 3), nil)));
    test_cell("simple_list", c, "(1 2 3)");
}

static void test_nested_list(US* us)
{
    Cell* c23 = cell_cons(us, cell_create_int(us, 2),
                    cell_cons(us, cell_create_int(us, 3), nil));
    Cell* c = cell_cons(us, cell_create_int(us, 1),
                  cell_cons(us, c23,
                            cell_cons(us, cell_create_int(us, 4), nil)));
    test_cell("nested_list", c, "(1 (2 3) 4)");
}

static void test_dotted_list(US* us)
{
    Cell* c = cell_cons(us, cell_create_int(us, 1),
                  cell_create_int(us, 2));
    test_cell("dotted_list", c, "(1 . 2)");
}

static void test_lists(US* us)
{
    test_simple_list(us);
    test_nested_list(us);
    test_dotted_list(us);
}

static void test_symbol(US* us)
{
    Env* parent = 0;
    Env* child = 0;
    do {
        parent = env_create(0);
        if (parent) {
            printf("ok symbol created parent env\n");
        } else {
            printf("BAD symbol created parent env\n");
            break;
        }

        const char* name = "set-gonzo!";

        Symbol* s1 = env_lookup(parent, name, 1);
        if (s1) {
            printf("ok symbol created sym [%s]\n", name);
        } else {
            printf("BAD symbol created sym [%s]\n", name);
            break;
        }

        const long value = 11;
        Cell* c = cell_create_int(us, value);
        if (c) {
            printf("ok symbol created cell [%ld]\n", c->ival);
        } else {
            printf("BAD symbol created cell [%ld] [%ld]\n", c->ival, value);
            break;
        }

        s1->value = c;

        Symbol* s2 = env_lookup(parent, name, 0);
        if (s2) {
            printf("ok symbol fetched sym [%s]\n", name);
        } else {
            printf("BAD symbol fetched sym [%s]\n", name);
            break;
        }

        child = env_create(0);
        if (child) {
            printf("ok symbol created child env\n");
        } else {
            printf("BAD symbol created child env\n");
            break;
        }

        env_chain(child, parent);

        Symbol* s3 = env_lookup(child, name, 0);
        if (s3) {
            printf("ok symbol fetched sym [%s]\n", name);
        } else {
            printf("BAD symbol fetched sym [%s]\n", name);
            break;
        }

        if (s3->value == c) {
            printf("ok symbol values match [%ld]\n", s3->value->ival);
        } else {
            printf("BAD symbol values match [%ld] [%ld]\n", s3->value->ival, value);
            break;
        }
    } while (0);
    env_dump(child, stderr);
    env_dump(parent, stderr);
    env_destroy(child);
    env_destroy(parent);
}

static void test_parser(US* us)
{
    static struct {
        const char* code;
        const char* expected;
    } data[] = {
        { " () ", "()" },
        { " 11 ", "11" },
        { " -3.1415 ", "-3.141500" },
        {
            "(2 3)",
            "(2 3)"
        },
        {
            " ( (2 3) ) ",
            "((2 3))"
        },
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
        parser_parse(us, parser, code);
        Cell* c = parser_result(parser);
        test_cell("parse", c, expected);
    }
    parser_destroy(parser);
}

static void test_eval_simple(US* us)
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
        { "1150", " (* 2 (+ 3 (* 5 4)) (+ (* 5 2) 6 (- 5 3) (+ 4 3))) " },
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
        { "1966", " (set! gonzus (+ 1960 6)) " },
        { "1966", " gonzus " },
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
        { "11", " 11 " },
        { "2", " (if #f 1 2)" },
        { "\"Sane\"", " (if (= 7 11) \"Crazy!\" \"Sane\")  " },
        { "7", " (if (= \"abc\" \"abc\") (+ 3 4) (- 5 6)) " },
        { "\"Positive!\"", " (if (> 13 0)  \"Positive!\" \"Negative\")  " },
        { "\"Negative\"", " (if (> 13 20) \"Positive!\" \"Negative\")  " },
        { "\"Negative\"", " (if (< 13 0)  \"Positive!\" \"Negative\")  " },
        { "\"Positive!\"", " (if (< 13 20) \"Positive!\" \"Negative\")  " },
    };

    int n = sizeof(data) / sizeof(data[0]);
    for (int j = 0; j < n; ++j) {
        const char* code = data[j].code;
        const char* expected = data[j].expected;
        Cell* c = us_eval_str(us, code);
        test_cell("eval_simple", c, expected);
        us_gc(us);
    }
}

static void test_eval_complex(US* us)
{
    static struct {
        const char* expected;
        const char* code;
    } data[] = {
        { "#t", "(define gonzo_t #t)" },
        { "#f", "(define gonzo_f #f)" },
        { "()", "(define nil (quote ()))" },
        { "<*CODE*>", "(define null? (lambda (L) (= L nil)))" },
        { "#t", "(null? (quote ()))" },
        { "#t", "(null? nil)" },
        { "(1 2 3 4)", "(define L (quote (1 2 3 4)))" },
        { "#f", "(null? L)" },
        { "<*CODE*>", "(define length (lambda (L) (if (= L nil) 0 (+ 1 (length (cdr L))))))" },
        { "4", "(length L)" },
        { "<*CODE*>", "(define empty? (lambda (L) (= 0 (length L))))" },
        { "<*CODE*>", "(define baz (lambda (x) x))" },
        { "4", "(baz 4)" },
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
        { "55", " (fib 10) " },
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

    int n = sizeof(data) / sizeof(data[0]);
    for (int j = 0; j < n; ++j) {
        const char* code = data[j].code;
        const char* expected = data[j].expected;
        Cell* c = us_eval_str(us, code);
        test_cell("eval_complex", c, expected);
        us_gc(us);
    }
}

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    US* us = us_create();

    test_arena();
    test_globals();
    test_strings(us);
    test_integers(us);
    test_reals(us);
    test_lists(us);
    test_symbol(us);
    test_parser(us);
    test_eval_simple(us);
    test_eval_complex(us);

    us_destroy(us);
    return 0;
}

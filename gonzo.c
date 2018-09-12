#include <stdio.h>
#include "cell.h"
#include "parser.h"
#include "env.h"
#include "us.h"

static void test_nil(void)
{
    printf("%s => ", "()");
    cell_print(nil, stdout, 1);
}

static void test_string(void)
{
    Cell* c = cell_create_string("In a hole in the ground there lived a hobbit...", 0);
    cell_print(c, stdout, 1);
}

static void test_number(void)
{
    static struct {
        int value;
        const char* expected;
    } data[] = {
        { 1984, "1984" },
        { 2001, "2001" },
    };

    for (int j = 0; j < sizeof(data) / sizeof(data[0]); ++j) {
        Cell* c = cell_create_int(data[j].value);
        printf("%s => ", data[j].expected);
        cell_print(c, stdout, 1);
    }

    Cell* ci = cell_create_int_from_string("-123456", 0);
    cell_print(ci, stdout, 1);

    Cell* cr = cell_create_real_from_string("-123.4567", 0);
    cell_print(cr, stdout, 1);
}

static void test_simple_list(void)
{
    Cell* c = cell_cons(cell_create_int(1),
                        cell_cons(cell_create_int(2),
                                  cell_cons(cell_create_int(3), nil)));
    printf("%s => ", "(1 2 3)");
    cell_print(c, stdout, 1);
}

static void test_nested_list(void)
{
    Cell* c23 = cell_cons(cell_create_int(2),
                          cell_cons(cell_create_int(3), nil));
    Cell* c = cell_cons(cell_create_int(1),
                        cell_cons(c23,
                                  cell_cons(cell_create_int(4), nil)));
    printf("%s => ", "(1 (2 3) 4)");
    cell_print(c, stdout, 1);
}

static void test_dotted(void)
{
    Cell* c = cell_cons(cell_create_int(1),
                        cell_create_int(2));
    printf("%s => ", "(1 . 2)");
    cell_print(c, stdout, 1);
}

static void test_symbol(void)
{
    const char* name = "set-gonzo!";
    Env* parent = env_create(0);
    printf("Created parent %p\n", parent);
    fflush(stdout);
    {
        Symbol* s = env_lookup(parent, name, 1);
        Cell* c = cell_create_int(11);
        s->value = c;
        printf("Inserted symbol [%s] in parent => %p\n", name, s);
        fflush(stdout);
    }
    {
        Symbol* s = env_lookup(parent, name, 0);
        printf("Looked up symbol [%s] in parent => %p\n", name, s);
        fflush(stdout);
    }

    Env* child = env_create(0);
    env_chain(child, parent);
    printf("Created child %p and chained to parent %p\n", child, parent);
    fflush(stdout);
    {
        Symbol* s = env_lookup(child, name, 0);
        printf("Looked up symbol [%s] in child => %p\n", name, s);
        fflush(stdout);
    }
}

static void test_parse(void)
{
    static struct {
        const char* code;
    } data[] = {
        { " () " },
        { " 11 " },
        { " -3.1415 " },
        { " ( 1\"hi\"2 3.3\"ho\"4.4 (2 3)\"hu\"(6 7) ) " },
        { " ( +1 -2 3. 4. +5.5 -6.6 +.7 .8 -.9 #t #f () a b c ) " },
        { " ( + - * / . % ! @ # $ ^ & ) " },
        { " \"The Hobbit rules!\" " },
        { " (1 2 3) " },
        { " (1 4+ 5- 6.1* 7.4/ 1.2.3 2 3 4.655 3[6]) " },
        { " (1 2 (a b c) 3 (4 x (5 y))) " },
        { "  ( define    fact   (lambda x (if(< x 2) 1 (* x(fact(1- x))))))  " },
        { "(define       (sum n) (if (zero? n  )         0 (+ n (sum (sub1 n)))))"       },
    };

    Parser* parser = parser_create(0);
    for (int j = 0; j < sizeof(data) / sizeof(data[0]); ++j) {
        const char* code = data[j].code;
        printf("===== Parsing [%s]:\n", code);
        fflush(stdout);
        parser_parse(parser, code);
        Cell* c = parser_result(parser);
        if (!c) {
            continue;
        }
        printf("[%p] => ", c);
        cell_print(c, stdout, 1);
    }
    parser_destroy(parser);
}

static void test_eval(void)
{
    static struct {
        const char* code;
    } data[] = {
        { " 11 " },
        { " () " },
        { " -3.1415 " },
        { " #t " },
        { " #f " },
        { " (+ 3 4) " },
        { " (+ 1 2 3 4) " },
        { " (+ 3 (+ 4 5) (+ 1 2)) " },
        { " (- 9) " },
        { " (- 9 4) " },
        { " (- 9 4 2) " },
        { " (- 9.0) " },
        { " (- 9.0 4.0) " },
        { " (- 9.0 4.0 2.0) " },
        { " (- 9 4.0) " },
        { " (- 9 4.0 2) " },
        { " (- 9 4.0 2.0) " },
        { " (- 9 4) " },
        { " (- 9 4 2.0) " },
        { " (* 2 3) " },
        { " (* 1 2 3 4) " },
        { " (* 2 (* 3 4) (* 5 6)) " },
        { " (* 2 (+ 3 (* 5 4)) (+ (* 5 2) 6)) " },
        { " (+ 3 4.5) " },
        { " (+ 3.5 4.5) " },
        { " (+) " },
        { " (-) " },
        { " (* 3 4.5) " },
        { " (* 3.5 4.5) " },
        { " (*) " },
        { " (quote (+ 3 4)) " },
        { " (define gonzus 11) " },
        { " gonzus " },
        { " (set! gonzus (* 7 11)) " },
        { " gonzus " },
        { " (set! invalid (+ 2 3)) " },
        { " invalid " },
        { " (=) " },
        { " (= 7) " },
        { " (= 7 11) " },
        { " (= 11 11) " },
        { " (= 7 11.0) " },
        { " (= 7.0 11) " },
        { " (= 7. 11.0) " },
        { " (= 7.0 7.0) " },
        { " (= 7 11 8) " },
        { " (= 7 7 7) " },
        { " (= \"Bilbo\" \"Bilbo\") " },
        { " (= \"Bilbo\" \"Frodo\") " },
        { " (= + +) " },
        { " (= + *) " },
        { " (if (= 7 11) \"Crazy!\" \"Sane\")  " },
        { " (if (= \"abc\" \"abc\") (+ 3 4) (- 5 6)) " },
        { " (if (> 13 0)  \"Positive!\" \"Negative\")  " },
        { " (if (> 13 20) \"Positive!\" \"Negative\")  " },
        { " (if (< 13 0)  \"Positive!\" \"Negative\")  " },
        { " (if (< 13 20) \"Positive!\" \"Negative\")  " },
        { " (define positive (lambda (x) (> x 0))) " },
        { " (positive  7) " },
        { " (positive -7) " },
        { " (define fact (lambda (n) (if (< n 2) 1 (* n (fact (- n 1)))))) " },
        { " (fact 5) " },
        { " (fact 10) " },
        { " (fact 20) " },
        { " (define fib (lambda (n) (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2)))))) " },
        { " (fib 4) " },
        { " (/) " },
        { " (/ 0) " },
        { " (/ 0.0) " },
        { " (/ 3 0) " },
        { " (/ 3 0.0) " },
        { " (/ 3.0 0) " },
        { " (/ 3.0 0.0) " },
        { " (/ 1) " },
        { " (/ -1) " },
        { " (/ 1.0) " },
        { " (/ -1.0) " },
        { " (/ 2) " },
        { " (/ -2) " },
        { " (/ 2.0) " },
        { " (/ -2.0) " },
        { " (/ 24 4) " },
        { " (/ 24 5) " },
        { " (/ 24.0 4) " },
        { " (/ 24.0 5) " },
        { " (/ 24 4.0) " },
        { " (/ 24 5.0) " },
        { " (/ 24.0 4.0) " },
        { " (/ 24.0 5.0) " },
        { " (/ 24 4 3) " },
        { " (/ 24 4 5) " },
        { " (/ 24.0 4 3) " },
        { " (/ 24.0 4 5) " },
        { " (/ 24 4.0 3) " },
        { " (/ 24 4.0 5) " },
        { " (/ 24.0 4.0 3) " },
        { " (/ 24.0 4.0 5) " },
        { " (/ 24 4 3.0) " },
        { " (/ 24 4 5.0) " },
        { " (/ 24.0 4 3.0) " },
        { " (/ 24.0 4 5.0) " },
        { " (/ 24 4.0 3.0) " },
        { " (/ 24 4.0 5.0) " },
        { " (/ 24.0 4.0 3.0) " },
        { " (/ 24.0 4.0 5.0) " },
        { " (car (quote (bofur bombur))) " },
        { " (cdr (quote (bofur bombur))) " },
        { " (cons (quote bifur) (quote (bofur bombur))) " },
        { " (define make-account (lambda (balance) (lambda (amt) (begin (set! balance (+ balance amt)) balance)))) " },
        { " (define acct1 (make-account 100.0)) " },
        { " (acct1 -20.0) " },
        { " (define acct2 (make-account 200.0)) " },
        { " (acct2 -20.0) " },
        { " (acct1 -10.0) " },
        { " (acct2 -20.0) " },
    };

    US* us = us_create();
    for (int j = 0; j < sizeof(data) / sizeof(data[0]); ++j) {
        const char* code = data[j].code;
        us_eval_str(us, code);
    }
    us_destroy(us);
}

static void test_repl(void)
{
    US* us = us_create();
    us_repl(us);
    us_destroy(us);
}

int main(int argc, char* argv[])
{
    int run_tests = argc <= 1;
    int run_repl  = argc >  1;

    if (run_tests) {
        test_nil();
        test_string();
        test_number();
        test_simple_list();
        test_nested_list();
        test_dotted();
        test_symbol();
        test_parse();
        test_eval();
    }

    if (run_repl) {
        test_repl();
    }

    return 0;
}

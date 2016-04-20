#include <stdio.h>
#include "cell.h"
#include "env.h"
#include "parser.h"

static void test_nil(void)
{
    printf("%s => ", "nil");
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
    Env* env0 = env_create(0, 0);
    printf("Created env0 %p\n", env0);
    {
        Symbol* s = env_lookup(env0, name, 1);
        Cell* c = cell_create_int(11);
        s->value = c;
        printf("Inserted symbol [%s] in env0 => %p\n", name, s);
    }
    {
        Symbol* s = env_lookup(env0, name, 0);
        printf("Looked up symbol [%s] in env0 => %p\n", name, s);
    }

    Env* env1 = env_create(0, env0);
    printf("Created env1 %p\n", env1);
    {
        Symbol* s = env_lookup(env1, name, 0);
        printf("Looked up symbol [%s] in env1 => %p\n", name, s);
    }
}

static void test_parse(void)
{
    static struct {
        const char* code;
    } data[] = {
        { " nil " },
        { " 11 " },
        { " -3.1415 " },
        { " ( +1 -2 3. 4. +5.5 -6.6 ) " },
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
        printf("Parsing [%s]:\n", code);
        parser_parse(parser, code);
        Cell* c = parser_result(parser);
        cell_print(c, stdout, 1);
    }
    parser_destroy(parser);
}

int main(int argc, char* argv[])
{
    printf("== START ==========\n");
    test_nil();
    test_string();
    test_number();
    test_simple_list();
    test_nested_list();
    test_dotted();
    test_symbol();
    test_parse();
    printf("== END ============\n");

    return 0;
}

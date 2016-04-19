#include "cell.h"
#include "env.h"

static void test_nil(void)
{
    printf("%s => ", "nil");
    cell_print(nil, stdout, 1);
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

int main(int argc, char* argv[])
{
    test_nil();
    test_number();
    test_simple_list();
    test_nested_list();
    test_dotted();
    test_symbol();

    return 0;
}

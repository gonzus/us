#include <stdlib.h>
#include <string.h>
#include "cell.h"
#include "env.h"
#include "parser.h"
#include "native.h"
#include "eval.h"
#include "us.h"

#if !defined(MEM_DEBUG)
#define MEM_DEBUG 1
#endif
#include "mem.h"

// #define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"

typedef struct NativeData {
    const char* name;
    NativeFunc* func;
} NativeData;

static Env* make_global_env(void);

US* us_create(void) {
    US* us = 0;
    MEM_ALLOC_TYPE(us, 1, US);
    LOG(DEBUG, ("US: created"));
    us->env = make_global_env();
    us->parser = parser_create(0);
    return us;
}

void us_destroy(US* us)
{
    parser_destroy(us->parser);
    env_unref(us->env);
    LOG(DEBUG, ("US: destroyed"));
    MEM_FREE_TYPE(us, 1, US);
}

const Cell* us_eval_str(US* us, const char* code)
{
    parser_parse(us->parser, code);
    Cell* c = parser_result(us->parser);
    LOG(INFO, ("=== parsed ==="));
    if (!c) {
        LOG(WARNING, ("Could not eval code [%s]", code));
        return 0;
    }

#if 0
    cell_print(c, stdout, 1);
#endif

    const Cell* r = 0;

#if 1
    r = cell_eval(c, us->env);
    LOG(INFO, ("=== evaled ==="));
#endif

#if 0
    printf("[%p] => ", r);
    cell_print(r, stdout, 1);
#endif

    return r;
}

void us_repl(US* us)
{
    while (1) {
        char buf[1024];
        fputs("> ", stdout);
        //buf[0] = '\0';
        if (!fgets(buf, 1024, stdin)) {
            break;
        }
#if 0
        printf("--> WILL PARSE: [%s]\n", buf);
        fflush(stdout);
#endif
        parser_parse(us->parser, buf);
        Cell* c = parser_result(us->parser);
        if (!c) {
            continue;
        }

#if 0
        printf("==> EVAL: ");
        cell_print(c, stdout, 1);
#endif

        const Cell* r = cell_eval(c, us->env);
        cell_print(r, stdout, 1);
    }
}

static Env* make_global_env(void)
{
    NativeData data[] = {
        { "+"       , func_add   },
        { "-"       , func_sub   },
        { "*"       , func_mul   },
        { "/"       , func_div   },
        { "="       , func_eq    },
        { ">"       , func_gt    },
        { "<"       , func_lt    },
        { "cons"    , func_cons  },
        { "car"     , func_car   },
        { "cdr"     , func_cdr   },
        { "begin"   , func_begin },
    };
    Env* env = env_create(0);
    env_ref(env);
    int n = sizeof(data) / sizeof(data[0]);
    for (int j = 0; j < n; ++j) {
        const char* name = data[j].name;
        Symbol* sym = env_lookup(env, name, 1);
        sym->value = cell_create_native(name, data[j].func);
        LOG(DEBUG, ("US: registered native handler for [%s]", name));
    }
    LOG(INFO, ("US: registered all native handlers"));
    return env;
}

#include <stdlib.h>
#include <string.h>
#include "arena.h"
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

static Env* make_global_env(US* us);

US* us_create(void) {
    US* us = 0;
    MEM_ALLOC_TYPE(us, 1, US);
    LOG(INFO, ("US: created at %p", us));
    us->arena = arena_create();
    us->parser = parser_create(0);
    us->env = make_global_env(us);
    return us;
}

void us_destroy(US* us)
{
    LOG(INFO, ("US: destroying %p", us));
    env_destroy(us->env);
    parser_destroy(us->parser);
    arena_destroy(us->arena);
    MEM_FREE_TYPE(us, 1, US);
}

static int mark_cell(US* us, const Cell* cell)
{
    if (!cell) {
        return 0;
    }
    if (arena_cell_used(us->arena, cell)) {
        fprintf(stderr, "=== MARKING already marked\n");
        return 0;
    }
    int count = 0;
    arena_mark_cell_used(us->arena, cell, 0);
    ++count;
    switch (cell->tag) {
        case CELL_CONS:
            fprintf(stderr, "=== MARKING cons\n");
#if 0
            for (const Cell* p = cell->cons.car; p && p != nil; p = p->cons.cdr) {
                count += mark_cell(us, p);
            }
#else
            count += mark_cell(us, cell->cons.car);
            count += mark_cell(us, cell->cons.cdr);
#endif
            break;
        case CELL_PROC:
            fprintf(stderr, "=== MARKING proc\n");
            count += mark_cell(us, cell->pval.params);
            count += mark_cell(us, cell->pval.body);
            break;
    }
    return count;
}

int us_gc(US* us)
{
    int count = 0;
    arena_reset_to_empty(us->arena);
    for (Env* env = us->env; env; env = env->parent) {
        for (int j = 0; j < env->size; ++j) {
            for (Symbol* sym = env->table[j]; sym; sym = sym->next) {
                count += mark_cell(us, sym->value);
            }
        }
    }
    return count;
}

const Cell* us_eval_str(US* us, const char* code)
{
    parser_parse(us, us->parser, code);
    Cell* c = parser_result(us->parser);
    LOG(INFO, ("=== parsed ==="));
    if (!c) {
        LOG(WARNING, ("Could not eval code [%s]", code));
        return 0;
    }

#if 0
    cell_print(c, stderr, 1);
#endif

    const Cell* r = 0;

#if 1
    r = cell_eval(us, c, us->env);
    LOG(INFO, ("=== evaled ==="));
#endif

#if 0
    printf("[%p] => ", r);
    cell_print(r, stderr, 1);
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
        parser_parse(us, us->parser, buf);
        Cell* c = parser_result(us->parser);
        if (!c) {
            continue;
        }

#if 0
        printf("==> EVAL: ");
        cell_print(c, stdout, 1);
#endif

        const Cell* r = cell_eval(us, c, us->env);
        cell_print(r, stdout, 1);
    }
}

static Env* make_global_env(US* us)
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
    int n = sizeof(data) / sizeof(data[0]);
    LOG(INFO, ("US: registering all %d native handlers, us %p, arena %p", n, us, us->arena));
    for (int j = 0; j < n; ++j) {
        const char* name = data[j].name;
        Symbol* sym = env_lookup(env, name, 1);
        sym->value = cell_create_native(us, name, data[j].func);
        LOG(DEBUG, ("US: registered native handler for [%s]", name));
    }
    LOG(INFO, ("US: registered all %d native handlers, us %p, arena %p", n, us, us->arena));
    arena_dump(us->arena, stderr);
    return env;
}

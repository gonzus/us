#include "arena.h"
#include "cell.h"
#include "env.h"
#include "parser.h"
#include "native.h"
#include "eval.h"
#include "us.h"

#if !defined(MEM_DEBUG)
#define MEM_DEBUG 0
#endif
#include "mem.h"

// #define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"

typedef struct NativeData {
    const char* name;
    NativeFunc* func;
} NativeData;

static Env* make_global_env(US* us);
static void mark_cell(US* us, const Cell* cell);
static void mark_env(US* us, Env* env);

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
    // env_destroy(us->env);
    parser_destroy(us->parser);
    arena_destroy(us->arena);
    MEM_FREE_TYPE(us, 1, US);
}

static void mark_cell(US* us, const Cell* cell)
{
    if (!cell) {
        return;
    }
    if (arena_is_cell_used(us->arena, cell)) {
        LOG(DEBUG, ("=== MARKING cell already marked"));
        return;
    }
    LOG(DEBUG, ("=== MARKING cell"));
    arena_mark_cell_used(us->arena, cell);
    switch (cell->tag) {
        case CELL_CONS:
            LOG(DEBUG, ("=== MARKING cell cons"));
            mark_cell(us, cell->cons.car);
            mark_cell(us, cell->cons.cdr);
            break;
        case CELL_PROC:
            LOG(DEBUG, ("=== MARKING cell proc"));
            mark_cell(us, cell->pval.params);
            mark_cell(us, cell->pval.body);
            mark_env(us, cell->pval.env);
            break;
    }
}

static void mark_env(US* us, Env* env)
{
    if (!env) {
        return;
    }
    if (arena_is_env_used(us->arena, env)) {
        LOG(DEBUG, ("=== MARKING env already marked"));
        return;
    }
    LOG(DEBUG, ("=== MARKING env"));
    arena_mark_env_used(us->arena, env);
    for (int j = 0; j < env->size; ++j) {
        for (Symbol* sym = env->table[j]; sym; sym = sym->next) {
            mark_cell(us, sym->value);
        }
    }
    mark_env(us, env->parent);
}

int us_gc(US* us)
{
    int count = 0;
    arena_reset_to_empty(us->arena);
    for (Env* env = us->env; env; env = env->parent) {
        mark_env(us, env);
    }
    return count;
}

Cell* us_eval_str(US* us, const char* code)
{
    parser_parse(us, us->parser, code);
    Cell* c = parser_result(us->parser);
    LOG(DEBUG, ("=== parsed ==="));
    if (!c) {
        LOG(WARNING, ("Could not eval code [%s]", code));
        return 0;
    }

    Cell* r = cell_eval(us, c, us->env);
    LOG(DEBUG, ("=== evaled ==="));

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
        parser_parse(us, us->parser, buf);
        Cell* c = parser_result(us->parser);
        if (!c) {
            continue;
        }

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
    Env* env = arena_get_env(us->arena, 0);
    int n = sizeof(data) / sizeof(data[0]);
    LOG(INFO, ("US: registering all %d native handlers, us %p, arena %p", n, us, us->arena));
    for (int j = 0; j < n; ++j) {
        const char* name = data[j].name;
        Symbol* sym = env_lookup(env, name, 1);
        sym->value = cell_create_native(us, name, data[j].func);
        LOG(INFO, ("US: registered native handler for [%s]", name));
    }
    LOG(INFO, ("US: registered all %d native handlers, us %p, arena %p", n, us, us->arena));
    arena_dump(us->arena, stderr);
    return env;
}

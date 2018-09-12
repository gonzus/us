#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "log.h"

static const char* log_file = 0;
static int log_line = 0;

static int log_printf(int level, const char* fmt, va_list ap);

void log_where(const char* file, int line)
{
    log_file = file;
    log_line = line;
}

void log_debug(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_printf(LOG_LEVEL_DEBUG, fmt, ap);
    va_end(ap);
}

void log_info(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_printf(LOG_LEVEL_INFO, fmt, ap);
    va_end(ap);
}

void log_warning(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_printf(LOG_LEVEL_WARNING, fmt, ap);
    va_end(ap);
}

void log_error(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_printf(LOG_LEVEL_ERROR, fmt, ap);
    va_end(ap);
}

void log_fatal(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_printf(LOG_LEVEL_FATAL, fmt, ap);
    va_end(ap);
    abort();
}

static int log_printf(int level, const char* fmt, va_list ap)
{
    static const char* Level[LOG_LEVEL_LAST] = {
        "DBG",
        "INF",
        "WRN",
        "ERR",
        "FTL",
    };
    const char* str_level = "???";
    if (level >= 0 && level < LOG_LEVEL_LAST) {
        str_level = Level[level];
    }

    time_t tloc;
    struct tm tdat;
    time(&tloc);
    localtime_r(&tloc, &tdat);

    pid_t pid = getpid();

    fprintf(stderr, "[%s] %04d-%02d-%02d %02d:%02d:%02d %d - %s:%d - ",
            str_level,
            tdat.tm_year + 1900, tdat.tm_mon + 1, tdat.tm_mday,
            tdat.tm_hour, tdat.tm_min, tdat.tm_sec,
            (int) pid,
            log_file, log_line);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    return 0;
}

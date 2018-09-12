#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
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
    const char* str = "???";
    if (level >= 0 && level < LOG_LEVEL_LAST) {
        str = Level[level];
    }
    fprintf(stderr, "[%s] - %s:%d - ", str, log_file, log_line);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    return 0;
}

#ifndef LOG_H_
#define LOG_H_

#define LOG_LEVEL_DEBUG   0
#define LOG_LEVEL_INFO    1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_ERROR   3
#define LOG_LEVEL_FATAL   4
#define LOG_LEVEL_LAST    5

#if !defined(LOG_LEVEL)
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

// #define LOG_LEVEL LOG_LEVEL_DEBUG
// #define LOG_LEVEL LOG_LEVEL_INFO
// #define LOG_LEVEL LOG_LEVEL_WARNING
// #define LOG_LEVEL LOG_LEVEL_ERROR
// #define LOG_LEVEL LOG_LEVEL_FATAL

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(x) (log_where(__FILE__, __LINE__), log_debug x)
#else
#define LOG_DEBUG(x) do {} while (0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(x) (log_where(__FILE__, __LINE__), log_info x)
#else
#define LOG_INFO(x) do {} while (0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARNING
#define LOG_WARNING(x) (log_where(__FILE__, __LINE__), log_warning x)
#else
#define LOG_WARNING(x) do {} while (0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(x) (log_where(__FILE__, __LINE__), log_error x)
#else
#define LOG_ERROR(x) do {} while (0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_FATAL
#define LOG_FATAL(x) (log_where(__FILE__, __LINE__), log_fatal x)
#else
#define LOG_FATAL(x) do {} while (0)
#endif

#define LOG(l, x) LOG_##l(x)

void log_debug(const char* fmt, ...);
void log_info(const char* fmt, ...);
void log_warning(const char* fmt, ...);
void log_error(const char* fmt, ...);
void log_fatal(const char* fmt, ...);

void log_where(const char* file, int line);

#endif

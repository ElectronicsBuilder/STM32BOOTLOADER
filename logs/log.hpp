#ifndef __LOG_HPP
#define __LOG_HPP

#ifdef __cplusplus
extern "C" {
#endif



#include <stdint.h>
#include <stdarg.h>   // for va_start, va_end

typedef enum
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_NONE
} LogLevel;

void log_set_level(LogLevel level);
void log_debug(const char* format, ...);
void log_info(const char* format, ...);
void log_warn(const char* format, ...);
void log_error(const char* format, ...);
void log_cmd(const char* format, ...);

#define LOG_DEBUG(...) log_debug(__VA_ARGS__)
#define LOG_INFO(...)  log_info(__VA_ARGS__)
#define LOG_WARN(...)  log_warn(__VA_ARGS__)
#define LOG_ERROR(...) log_error(__VA_ARGS__)
#define LOG_CMD(...) log_error(__VA_ARGS__)
#ifdef __cplusplus
}
#endif

#endif /* __LOG_HPP */
#include "log.hpp"
#include "main.h"
#include <stdio.h>

static LogLevel current_log_level = LOG_DEBUG;

void log_set_level(LogLevel level)
{
    current_log_level = level;
}

static void log_output(const char* color_code, const char* level, const char* format, va_list args)
{
    printf("\r%s[%s] ", color_code, level);
    vprintf(format, args);
    printf("\033[0m\n");
}

static void log_input(const char* color_code, const char* level, const char* format, va_list args)
{
    printf("\r%s[%s] ", color_code, level);
    vprintf(format, args);
    printf("\033[0m\n");
}

void log_debug(const char* format, ...)
{
    if (current_log_level <= LOG_DEBUG)
    {
        va_list args;
        va_start(args, format);
        log_output("\033[36m", "Aidley >>", format, args);
        va_end(args);
    }
}

void log_info(const char* format, ...)
{
    if (current_log_level <= LOG_INFO)
    {
        va_list args;
        va_start(args, format);
        log_output("\033[32m", "INFO ", format, args);
        va_end(args);
    }
}

void log_warn(const char* format, ...)
{
    if (current_log_level <= LOG_WARN)
    {
        va_list args;
        va_start(args, format);
        log_output("\033[33m", "WARN ", format, args);
        va_end(args);
    }
}

void log_error(const char* format, ...)
{
    if (current_log_level <= LOG_ERROR)
    {
        // Turn on ERROR LED (active low)
        if (HAL_GPIO_ReadPin(LED_ERROR_GPIO_Port, LED_ERROR_Pin) != GPIO_PIN_RESET) {
            HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_RESET);
        }
        va_list args;
        va_start(args, format);
        log_output("\033[31m", "ERROR", format, args);
        va_end(args);
    }
}

void log_cmd(const char* format, ...)
{
    if (current_log_level <= LOG_DEBUG)
    {
        va_list args;
        va_start(args, format);
        log_input("\033[36m", "Aidley >>", format, args);
        va_end(args);
    }
}


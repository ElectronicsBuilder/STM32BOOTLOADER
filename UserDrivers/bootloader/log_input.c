// log_input.c
#include "log_input.h"
#include "uart.h"
#include "stm32f7xx_hal.h"
#include <string.h>

 extern UART_HandleTypeDef huart1;


static char log_buffer[128];
static size_t log_pos = 0;

void uart_handle_log_input(uint8_t byte) {
    switch (byte) {
        case '\r':
        case '\n':
            log_buffer[log_pos] = '\0';
            if (log_pos > 0) {
                log_pos = 0;
            }
            uart_send_string("\r\n\033[36m[Aidley >>] \033[0m");
            break;

        case 0x7F: // DEL / Backspace
        case '\b':
            if (log_pos > 0) {
                log_pos--;
                log_buffer[log_pos] = '\0';
                uart_send_string("\b \b");
            }
            break;

        default:
            if (log_pos < sizeof(log_buffer) - 1 && byte >= 0x20 && byte <= 0x7E) {
                log_buffer[log_pos++] = byte;
                HAL_UART_Transmit(&huart1, &byte, 1, HAL_MAX_DELAY);  // Echo character
            }
            break;
    }
}

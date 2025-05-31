#ifndef __UART_HPP
#define __UART_HPP

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>

typedef enum {
    UART_MODE_LOG_INPUT = 0,
    UART_MODE_BOOTLOADER_RX,
    UART_MODE_BOOTLOADER_DATA
} UART_RxMode;

extern UART_RxMode uart_rx_mode;

void uart_init_rx(); // call this once in system init
void uart_handle_log_input(uint8_t byte);
void uart_handle_bootloader_command(uint8_t byte);
void uart_handle_bootloader_binary(uint8_t byte);

void uart_init_rx_dma();
void uart_dma_poll();


#ifdef __cplusplus
}
#endif

#endif /* __UART_HPP */
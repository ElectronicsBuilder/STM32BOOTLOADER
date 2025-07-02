// #ifndef __UART_HPP
// #define __UART_HPP

// #ifdef __cplusplus
// extern "C" {
// #endif


// #include <stdint.h>

// typedef enum {
//     UART_MODE_LOG_INPUT = 0,
//     UART_MODE_BOOTLOADER_RX,
//     UART_MODE_BOOTLOADER_DATA
// } UART_RxMode;

// extern UART_RxMode uart_rx_mode;

// void uart_init_rx(); // call this once in system init
// void uart_handle_log_input(uint8_t byte);
// void uart_handle_bootloader_command(uint8_t byte);
// void uart_handle_bootloader_binary(uint8_t byte);

// void uart_init_rx_dma();
// void uart_dma_poll();

// void uart_command_queue_init(void);
// void uart_binary_queue_init(void);

// void uart_send_string(const char *msg);

// #ifdef __cplusplus
// }
// #endif

// #endif /* __UART_HPP */

#ifndef __UART_HPP
#define __UART_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "boot_defs.h"
// typedef enum {
//     UART_MODE_LOG_INPUT = 0,
//     UART_MODE_BOOTLOADER_RX,
//     UART_MODE_BOOTLOADER_DATA
// } UART_RxMode;


extern UART_RxMode uart_rx_mode;

// Setup
void uart_set_mode(UART_RxMode mode);
void uart_set_transport_mode(boot_uart_mode_t mode);

// Transport interface
bool uart_data_available(void);
int  uart_read_buffer(uint8_t *buf, size_t len);
void uart_send_string(const char *msg);
void uart_send_bytes(const uint8_t *data, size_t len);

// Internal
void uart_init_rx();
void uart_init_rx_dma();
void uart_dma_poll();

// Optional
void uart_command_queue_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __UART_HPP */

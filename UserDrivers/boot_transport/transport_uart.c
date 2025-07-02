#include "transport_uart.h"
#include "uart.h"
#include "boot_defs.h"
#include "log_input.h"
#include "command_parser.h"
#include "binary_loader.h"
#include "stm32f7xx_hal.h"
#include <string.h>

extern UART_HandleTypeDef huart1;


static boot_uart_mode_t uart_mode = BOOT_UART_MODE_SELECTED;

uint8_t chunk_buf[BOOT_BINARY_CHUNK_SIZE];
static size_t chunk_pos = 0;

static void uart_transport_init(void) {
    uart_set_transport_mode(uart_mode);
    uart_set_mode(UART_MODE_BOOTLOADER_RX);//UART_MODE_BOOTLOADER_RX
}

static bool uart_transport_poll(void) {
    uint8_t byte;

    while (uart_read_buffer(&byte, 1) == 1) {
        switch (uart_rx_mode) {
            case UART_MODE_LOG_INPUT:
                uart_handle_log_input(byte);
                break;

            case UART_MODE_BOOTLOADER_RX:
                process_packet_byte(byte);
                
                break;

            case UART_MODE_BOOTLOADER_DATA:
                chunk_buf[chunk_pos++] = byte;
                if (chunk_pos == BOOT_BINARY_CHUNK_SIZE) {
                    process_binary_chunk(chunk_buf, chunk_pos);
                    chunk_pos = 0;
                }
                break;

            case UART_MODE_BOOTLOADER_EXT_MEM:
                chunk_buf[chunk_pos++] = byte;
                if (chunk_pos == BOOT_EXT_MEM_CHUNK_SIZE) {
                    process_external_memory_chunk(chunk_buf, chunk_pos);
                    chunk_pos = 0;
                }
                break;
        }
    }

    return true;
}

static int uart_transport_read(uint8_t *buf, size_t len) {
    return uart_read_buffer(buf, len);
}

static int uart_transport_write(const uint8_t *buf, size_t len) {
    return (HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, HAL_MAX_DELAY) == HAL_OK) ? (int)len : -1;
}

static void uart_transport_flush(void) {
    // no-op for now
}

const BootTransportDriver uart_transport_driver = {
    .init  = uart_transport_init,
    .poll  = uart_transport_poll,
    .read  = uart_transport_read,
    .write = uart_transport_write,
    .flush = uart_transport_flush
};

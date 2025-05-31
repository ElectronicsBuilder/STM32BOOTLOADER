#include "transport_uart.h"
#include "uart.h"        // Your existing mode and ISR logic
#include "boot_defs.h"
#include "stm32f7xx_hal.h"
#include <string.h>

// Configurable handle
extern UART_HandleTypeDef huart1;

static boot_uart_mode_t uart_mode = BOOT_UART_MODE_SELECTED;

static void uart_transport_init(void) {
    uart_rx_mode = UART_MODE_BOOTLOADER_RX;

    switch (uart_mode) {
        case UART_MODE_SIMPLE:
            // No-op, HAL handles on demand
            break;
        case UART_MODE_IRQ:
        case UART_MODE_DMA:
            uart_init_rx();  // Your ISR-based buffer start
            break;
    }
}

static bool uart_transport_poll(void) {
    return uart_data_available();  // Your ring buffer status check
}

static int uart_transport_read(uint8_t *buf, size_t len) {
    return uart_read_buffer(buf, len);  // Pulls from ring buffer or blocking
}

static int uart_transport_write(const uint8_t *buf, size_t len) {
    return (HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, HAL_MAX_DELAY) == HAL_OK) ? (int)len : -1;
}

static void uart_transport_flush(void) {
    // Placeholder for future TX flush or DMA sync
}

const BootTransportDriver boot_uart_driver = {
    .init  = uart_transport_init,
    .poll  = uart_transport_poll,
    .read  = uart_transport_read,
    .write = uart_transport_write,
    .flush = uart_transport_flush
};

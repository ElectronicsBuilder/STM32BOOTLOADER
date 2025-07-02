#include "uart.h"
#include "log.h"
#include "stm32f7xx_hal.h"
#include "boot_defs.h"
#include <string.h>
#include <stdbool.h>

#if BOOTLOADER_USE_RTOS
#include "FreeRTOS.h"
#include "queue.h"
QueueHandle_t uart_cmd_queue;
#endif

extern UART_HandleTypeDef huart1;

UART_RxMode uart_rx_mode = UART_MODE_BOOTLOADER_DATA;
static UART_RxMode current_rx_mode = UART_MODE_BOOTLOADER_DATA;
static boot_uart_mode_t uart_mode = UART_MODE_SIMPLE;

static const char* prompt = "\r\n\033[36m[Aidley >>] \033[0m";

#define UART_RING_SIZE 2048

// === Ring Buffer ===
static uint8_t uart_ring_data[UART_RING_SIZE];
static size_t ring_head = 0;
static size_t ring_tail = 0;

static bool ring_buffer_empty(void) {
    return ring_head == ring_tail;
}

static bool ring_buffer_write(uint8_t byte) {
    size_t next_head = (ring_head + 1) % UART_RING_SIZE;
    if (next_head == ring_tail) return false;  // overflow
    uart_ring_data[ring_head] = byte;
    ring_head = next_head;
    return true;
}

static bool ring_buffer_read(uint8_t *out) {
    if (ring_buffer_empty()) return false;
    *out = uart_ring_data[ring_tail];
    ring_tail = (ring_tail + 1) % UART_RING_SIZE;
    return true;
}

#define UART_DMA_BUFFER_SIZE 2048
static uint8_t uart_dma_rx_buf[UART_DMA_BUFFER_SIZE];
static volatile uint16_t last_pos = 0;

void uart_set_mode(UART_RxMode mode) {
    current_rx_mode = mode;
    uart_rx_mode = mode;
}

void uart_set_transport_mode(boot_uart_mode_t mode) {
    uart_mode = mode;
    if (mode == UART_MODE_IRQ) {
        ring_head = ring_tail = 0;
        uart_init_rx();
    } else if (mode == UART_MODE_DMA) {
        uart_init_rx_dma();
    }
}

bool uart_data_available(void) {
    switch (uart_mode) {
        case UART_MODE_IRQ:
            return !ring_buffer_empty();
        case UART_MODE_DMA:
            return true;
        case UART_MODE_SIMPLE:
            return true;
        default:
            return false;
    }
}

int uart_read_buffer(uint8_t *buf, size_t len) {
    if (!buf || len == 0) return 0;
    int read = 0;

    switch (uart_mode) {
        case UART_MODE_IRQ:
            while (read < len && ring_buffer_read(&buf[read]))
                read++;
            return read;

        case UART_MODE_DMA: {
            uint16_t current_pos = UART_DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx);
            while (last_pos != current_pos && read < len) {
                buf[read++] = uart_dma_rx_buf[last_pos++];
                if (last_pos >= UART_DMA_BUFFER_SIZE)
                    last_pos = 0;
            }
            return read;
        }

        case UART_MODE_SIMPLE:
            if (HAL_UART_Receive(&huart1, buf, len, HAL_MAX_DELAY) == HAL_OK)
                return len;
            else
                return 0;

        default:
            return 0;
    }
}

void uart_send_string(const char *msg) {
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

void uart_send_bytes(const uint8_t *data, size_t len) {
    HAL_UART_Transmit(&huart1, (uint8_t *)data, len, HAL_MAX_DELAY);
}

static uint8_t rx_byte = 0;
void uart_init_rx() {
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1 && uart_mode == UART_MODE_IRQ) {
        ring_buffer_write(rx_byte);
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}

void uart_init_rx_dma() {
    last_pos = 0;
    HAL_UART_Receive_DMA(&huart1, uart_dma_rx_buf, UART_DMA_BUFFER_SIZE);
    HAL_UART_Transmit(&huart1, (uint8_t*)prompt, strlen(prompt), HAL_MAX_DELAY);
}

void uart_dma_poll() {
    // Not used in new design; handled by uart_read_buffer
}

void uart_command_queue_init(void) {
#if BOOTLOADER_USE_RTOS
    uart_cmd_queue = xQueueCreate(64, sizeof(uint8_t));
#endif
}










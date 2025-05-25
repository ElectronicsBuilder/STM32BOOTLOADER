// uart.cpp
#include "uart.hpp"
//#include "bootloader.h"
#include "log.hpp"
#include <string.h>
#include "stm32f7xx_hal.h"


extern UART_HandleTypeDef huart1;

UART_RxMode uart_rx_mode = UART_MODE_LOG_INPUT;
const char* prompt = "\r\n\033[36m[Aidley >>] \033[0m";


// === Interrupt-Based RX ===
static uint8_t rx_byte = 0;

void uart_init_rx()
{
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        switch (uart_rx_mode)
        {
            case UART_MODE_LOG_INPUT:
                uart_handle_log_input(rx_byte);
                break;

            case UART_MODE_BOOTLOADER_RX:
                uart_handle_bootloader_command(rx_byte);
                break;

            case UART_MODE_BOOTLOADER_DATA:
                uart_handle_bootloader_binary(rx_byte);
                break;
        }
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}

// === DMA-Based RX ===
#define UART_DMA_BUFFER_SIZE 256
static uint8_t uart_dma_rx_buf[UART_DMA_BUFFER_SIZE];
static volatile uint16_t last_pos = 0;

void uart_init_rx_dma()
{
    last_pos = 0;
   
    HAL_UART_Receive_DMA(&huart1, uart_dma_rx_buf, UART_DMA_BUFFER_SIZE);
    HAL_UART_Transmit(&huart1, (uint8_t*)prompt, strlen(prompt), HAL_MAX_DELAY);
}

void uart_dma_poll()
{
    uint16_t current_pos = UART_DMA_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx);
    while (last_pos != current_pos)
    {
        uint8_t byte = uart_dma_rx_buf[last_pos++];
        if (last_pos >= UART_DMA_BUFFER_SIZE)
            last_pos = 0;

        switch (uart_rx_mode)
        {
            case UART_MODE_LOG_INPUT:
                uart_handle_log_input(byte);
                break;

            case UART_MODE_BOOTLOADER_RX:
                uart_handle_bootloader_command(byte);
                break;

            case UART_MODE_BOOTLOADER_DATA:
                uart_handle_bootloader_binary(byte);
                break;
        }
    }
}

// === Handlers ===
void uart_handle_log_input(uint8_t byte)
{
    static char buffer[100];
    static uint32_t pos = 0;
    //const char* prompt = "\r\n\033[36m[Aidley >>] \033[0m";

    switch (byte)
    {
        case '\r':
        case '\n':

            buffer[pos] = '\0';

            if (pos > 0) {
                pos = 0;
            }
            HAL_UART_Transmit(&huart1, (uint8_t*)prompt, strlen(prompt), HAL_MAX_DELAY);
            break;

        case 0x7F: // backspace (DEL key)
        case '\b': // backspace
            if (pos > 0) {
                pos--;
                buffer[pos] = '\0';
                HAL_UART_Transmit(&huart1, (uint8_t*)"\b \b", 3, HAL_MAX_DELAY);
            }
            break;
      

        default:
            if (pos < sizeof(buffer) - 1 && byte >= 0x20 && byte <= 0x7E) {
                buffer[pos++] = byte;
                
                HAL_UART_Transmit(&huart1, &byte, 1, HAL_MAX_DELAY); // echo typed char
            }
            break;
    }
}

void uart_handle_bootloader_command(uint8_t byte)
{
    // TODO: Implement based on your previous command buffer logic
}

void uart_handle_bootloader_binary(uint8_t byte)
{
    // TODO: Implement based on RAM_Buf, block, CRC etc.
}

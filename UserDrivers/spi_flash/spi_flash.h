#ifndef SPI_FLASH_H
#define SPI_FLASH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "main.h"

#if defined(STM32F4xx)
    #include "stm32f4xx_hal.h"
#elif defined(STM32F7xx)
    #include "stm32f7xx_hal.h"
#else
    #error "Unsupported STM32 family: please define STM32F4xx or STM32F7xx"
#endif



typedef struct {
    SPI_HandleTypeDef* spiHandle;
    GPIO_TypeDef* csPort;
    uint16_t csPin;
} SpiFlash;

typedef struct {
    const char* name;
    uint32_t size_mbit;
    uint32_t page_size;
    uint32_t sector_size;
    bool quad_enabled;
} FlashDeviceInfo;

void spi_flash_init(SpiFlash* flash, SPI_HandleTypeDef* spiHandle, GPIO_TypeDef* csPort, uint16_t csPin);

void spi_flash_select(SpiFlash* flash);
void spi_flash_deselect(SpiFlash* flash);
void spi_flash_send_command(SpiFlash* flash, uint8_t cmd);

void spi_flash_write_enable(SpiFlash* flash);
void spi_flash_write_data(SpiFlash* flash, uint32_t address, const uint8_t* data, size_t length);
void spi_flash_read_data(SpiFlash* flash, uint32_t address, uint8_t* buffer, size_t length);
void spi_flash_erase_sector(SpiFlash* flash, uint32_t address);
void spi_flash_poll_busy(SpiFlash* flash);

uint32_t spi_flash_read_device_id(SpiFlash* flash);
FlashDeviceInfo spi_flash_get_device_info(void);

void spi_flash_reset(SpiFlash* flash); // Reserved

#endif // SPI_FLASH_H

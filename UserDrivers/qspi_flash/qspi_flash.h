#ifndef QSPI_FLASH_H
#define QSPI_FLASH_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "main.h"

#if defined(STM32F4xx)
#include "stm32f4xx_hal.h"
#elif defined(STM32F7xx)
#include "stm32f7xx_hal.h"
#else
#error "Unsupported STM32 family"
#endif

typedef struct {
    const char* name;
    uint32_t size_mbit;
    uint32_t page_size;
    uint32_t sector_size;
    bool quad_enabled;
} QFlashDeviceInfo;

typedef struct {
    QSPI_HandleTypeDef* hqspi;
} QspiFlash;

extern volatile bool qspi_dma_tx_done;

void qspi_flash_init(QspiFlash* qf, QSPI_HandleTypeDef* handle);
void qspi_flash_reset(QspiFlash* qf);
void qspi_flash_read_id(QspiFlash* qf, uint8_t* idBuffer);
void qspi_flash_read(QspiFlash* qf, uint32_t addr, uint8_t* buf, size_t size);
void qspi_flash_read_quad(QspiFlash* qf, uint32_t addr, uint8_t* buf, size_t size);
void qspi_flash_write(QspiFlash* qf, uint32_t addr, const uint8_t* data, size_t size);
void qspi_flash_write_quad(QspiFlash* qf, uint32_t addr, const uint8_t* data, size_t size);
bool qspi_flash_write_quad_dma(QspiFlash* qf, uint32_t addr, const uint8_t* data, size_t size);
bool qspi_flash_read_quad_dma(QspiFlash* qf, uint32_t addr, uint8_t* buf, size_t size);
bool qspi_flash_wait_dma_complete(uint32_t timeout_ms);
void qspi_flash_erase_sector(QspiFlash* qf, uint32_t addr);
void qspi_flash_erase_chip(QspiFlash* qf);
void qspi_flash_set_quad_enable(QspiFlash* qf);
void qspi_flash_enable_mmap(QspiFlash* qf);
void qspi_flash_enable_quad_mmap(QspiFlash* qf);
void qspi_flash_enable_dual_mmap(QspiFlash* qf);
QFlashDeviceInfo qspi_flash_get_info(void);

#endif

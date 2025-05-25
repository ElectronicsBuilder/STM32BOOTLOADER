#ifndef __BOOT_DEFS_H
#define __BOOT_DEFS_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "main.h"

#if defined(STM32F4xx)
#include "stm32f4xx_hal.h"
#elif defined(STM32F7xx)
#include "stm32f7xx_hal.h"
#else
#error "Unsupported STM32 family"
#endif



#define APPLICATION_ADDRESS (uint32_t)0x08080000
#define BOOTLOADER_ADDRESS 0x08000000U


//Boot Fuse 
// Define which fuse backend to use
#define BOOT_FUSE_BACKEND_SPI_FLASH     1
#define BOOT_FUSE_BACKEND_QSPI_FLASH    2
#define BOOT_FUSE_BACKEND_EEPROM        3  // future

// SELECT ACTIVE DRIVER
#define BOOT_FUSE_BACKEND_SELECTED      BOOT_FUSE_BACKEND_SPI_FLASH



// Boot Fuse Metadata Format
#define BOOT_FUSE_MAGIC      0xB007F053
#define BOOT_FUSE_VERSION    1
#define BOOT_FUSE_DATA_LEN   3

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint8_t  version;
    uint8_t  len;
    uint8_t  fuse_data[BOOT_FUSE_DATA_LEN];
    uint8_t  crc8;
} boot_fuse_metadata_t;


uint8_t boot_crc8(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif


#endif // __BOOT_DEFS_H


























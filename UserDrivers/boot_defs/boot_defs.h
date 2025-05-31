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
#define BOOT_FUSE_BACKEND_SELECTED      BOOT_FUSE_BACKEND_QSPI_FLASH

//Boot Transport
#define BOOT_TRANSPORT_BACKEND_UART      1
#define BOOT_TRANSPORT_BACKEND_WIFI      2
#define BOOT_TRANSPORT_BACKEND_ETHERNET  3
#define BOOT_TRANSPORT_BACKEND_USB       4

#define BOOT_TRANSPORT_BACKEND_SELECTED  BOOT_TRANSPORT_BACKEND_UART



#if BOOT_TRANSPORT_BACKEND_SELECTED == BOOT_TRANSPORT_BACKEND_UART
    #include "transport_uart.h"
    #define ACTIVE_BOOT_TRANSPORT_DRIVER &boot_uart_driver

#elif BOOT_TRANSPORT_BACKEND_SELECTED == BOOT_TRANSPORT_BACKEND_USB
    #include "transport_usb.h"
    #define ACTIVE_BOOT_TRANSPORT_DRIVER &boot_usb_driver

#elif BOOT_TRANSPORT_BACKEND_SELECTED == BOOT_TRANSPORT_BACKEND_ETH
    #include "transport_eth.h"
    #define ACTIVE_BOOT_TRANSPORT_DRIVER &boot_eth_driver

#else
    #error "No valid BOOT_TRANSPORT_SELECTED defined"
#endif


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


// UART Transport Mode 

typedef enum {
    UART_MODE_SIMPLE = 0,
    UART_MODE_IRQ,
    UART_MODE_DMA
} boot_uart_mode_t;

#define BOOT_UART_MODE_SELECTED    UART_MODE_DMA  // Set default mode here






#ifdef __cplusplus
}
#endif


#endif // __BOOT_DEFS_H


























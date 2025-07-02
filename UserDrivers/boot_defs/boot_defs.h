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



#define APPLICATION_ADDRESS                     (uint32_t)0x08080000U
#define APPLICATION_END_ADDRESS                 ((uint32_t)0x08200000)
#define APPLICATION_MAX_SIZE                    (APPLICATION_END_ADDRESS - APPLICATION_ADDRESS)  // = 0x180000 (1.5MB)
#define QSPI_BASE_ADDR                          0x90000000UL
#define INTERNAL_FLASH_START                    APPLICATION_ADDRESS
#define INTERNAL_FLASH_END                      APPLICATION_END_ADDRESS
#define FLASH_START_SECTOR                      FLASH_SECTOR_6
#define FLASH_END_SECTOR                        FLASH_SECTOR_11
#define FLASH_SECTOR_SIZE                       0x20000U 

// User Space. Used in Driver Test 
#define QSPI_USERDATA_WRITE_ADDR                0x00C00000UL
#define QSPI_USERDATA_ADDR                      0x90C00000UL
#define QSPI_USERDATA_SIZE                      (1U * 1024U * 1024U)  // 1 MB

//Bootloader uses RTOS 

#define BOOTLOADER_USE_RTOS                     1U

// Adjust based on buffer/RAM
#define BOOT_BINARY_CHUNK_SIZE                  (1026) // 1024 + 2 crc bytes  
#define BOOT_EXT_MEM_CHUNK_SIZE                 (1026) // 1024 + 2 crc bytes  
//Boot Fuse  
// Define which fuse backend to use
#define BOOT_FUSE_BACKEND_SPI_FLASH             1U
#define BOOT_FUSE_BACKEND_QSPI_FLASH            2U
#define BOOT_FUSE_BACKEND_EEPROM                3U  // future

// SELECT ACTIVE DRIVER
#define BOOT_FUSE_BACKEND_SELECTED              BOOT_FUSE_BACKEND_QSPI_FLASH

//Boot Transport
#define BOOT_TRANSPORT_BACKEND_UART             1U
#define BOOT_TRANSPORT_BACKEND_WIFI             2U
#define BOOT_TRANSPORT_BACKEND_ETHERNET         3U
#define BOOT_TRANSPORT_BACKEND_USB              4U

#define BOOT_TRANSPORT_BACKEND_SELECTED         BOOT_TRANSPORT_BACKEND_UART



// Boot Fuse Metadata Format
#define BOOT_FUSE_MAGIC                         0xB007F053
#define BOOT_FUSE_VERSION                       1U
#define BOOT_FUSE_DATA_LEN                      3U

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
    UART_MODE_LOG_INPUT = 0,
    UART_MODE_BOOTLOADER_RX,
    UART_MODE_BOOTLOADER_DATA,
    UART_MODE_BOOTLOADER_EXT_MEM

} UART_RxMode;


typedef enum {
    UART_MODE_SIMPLE = 0,
    UART_MODE_IRQ,
    UART_MODE_DMA
} boot_uart_mode_t;

#define BOOT_UART_MODE_SELECTED                 UART_MODE_DMA  // Set default mode here




// --- Bootloader Framing Bytes ---
#define BOOT_CMD_STX                            0x7E  // Start of command packet
#define BOOT_CMD_ETX                            0x7F  // End of command packet

// --- Bootloader Command IDs ---
typedef enum {
    BOOT_CMD_GET_VERSION    = 0x01,
    BOOT_CMD_JUMP_TO_APP    = 0x02,
    BOOT_CMD_BOOTSTART      = 0x03,
    BOOT_CMD_COMMIT         = 0x04,
    BOOT_CMD_EXTMEM_INIT    = 0x05
    

    // Extend with more commands as needed
} BootCommand;

//Boot Staging 
#define QSPI_STAGING_ADDR                       0x00D00000UL
#define QSPI_STAGING_SIZE                       (2U * 1024U * 1024U)

typedef struct {
    uint32_t size;
    uint32_t crc;
} staging_binary_info_t;



#define BOOT_STAGING_BACKEND_QSPI_FLASH         1U
#define BOOT_STAGING_BACKEND_SPI_FLASH          2U
#define BOOT_STAGING_BACKEND_INTERNAL_FLASH     3U

#define BOOT_STAGING_BACKEND_SELECTED           BOOT_STAGING_BACKEND_QSPI_FLASH



#define QSPI_FLASH_SIZE_BYTES                   (16U * 1024U * 1024U)
#define QSPI_GUI_ADDR                           0x00000000UL // For APP 
#define QSPI_GUI_SIZE_MAX                       (13U * 1024U * 1024U)



#define QSPI_METADATA_ADDR                      0x00F00000UL
#define QSPI_METADATA_SIZE                      (1024U * 1024U)

#define QSPI_FUSE_ADDR                          0x00FFF000UL
#define QSPI_FUSE_SIZE                          0x00001000UL

#define QSPI_MEM_MAPPED_ADDR                    ((uint32_t)(QSPI_BASE_ADDR + QSPI_STAGING_ADDR))


//Boot commit 

// Boot Commit Backend Options (usually matches staging backend)
#define BOOT_COMMIT_BACKEND_QSPI_FLASH          1U
#define BOOT_COMMIT_BACKEND_SPI_FLASH           2U

#define BOOT_COMMIT_BACKEND_SELECTED            BOOT_COMMIT_BACKEND_QSPI_FLASH

//External Memory Writer 
#define EXTMEM_WRITER_BACKEND_QSPI_FLASH        1U 
#define EXTMEM_WRITER_BACKEND_FLASH             2U
#define EXTMEM_WRITER_BACKEND_SELECTED          EXTMEM_WRITER_BACKEND_QSPI_FLASH

typedef struct {
    uint32_t size;
    uint32_t crc;
} extmem_info_t;


//helper functions 

uint16_t compute_crc16( uint16_t crc,  uint8_t * BUFFER, long length);


#ifdef __cplusplus
}
#endif


#endif // __BOOT_DEFS_H


























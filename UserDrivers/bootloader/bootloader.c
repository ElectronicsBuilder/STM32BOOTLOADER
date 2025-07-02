// bootloader.c
#include "bootloader.h"
#include "boot_defs.h"


#if BOOT_STAGING_BACKEND_SELECTED == BOOT_STAGING_BACKEND_QSPI_FLASH || \
    BOOT_FUSE_BACKEND_SELECTED == BOOT_FUSE_BACKEND_QSPI_FLASH
#include "qspi_flash.h"
extern QSPI_HandleTypeDef hqspi;
extern QspiFlash qspiFlash;
#endif

#if BOOT_STAGING_BACKEND_SELECTED == BOOT_STAGING_BACKEND_SPI_FLASH || \
    BOOT_FUSE_BACKEND_SELECTED == BOOT_FUSE_BACKEND_SPI_FLASH
#include "spi_flash.h"
extern SpiFlash spiFlash;
extern SPI_HandleTypeDef hspi1;
#endif

void bootloader_driver_init(void)
{
#if BOOT_STAGING_BACKEND_SELECTED == BOOT_STAGING_BACKEND_QSPI_FLASH
    qspi_flash_init(&qspiFlash, &hqspi);  

    qspi_flash_set_quad_enable(&qspiFlash);

    qspi_flash_auto_poll_ready(&qspiFlash, 5000);

#endif

#if BOOT_STAGING_BACKEND_SELECTED == BOOT_STAGING_BACKEND_SPI_FLASH
     spi_flash_init(&spiFlash, &hspi1, FLASH_CS_GPIO_Port, FLASH_CS_Pin);

#endif

    // Add other backend driver inits here as needed
}

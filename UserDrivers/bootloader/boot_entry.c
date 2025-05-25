#include "boot_fuse.h"
#include "boot_defs.h"
#include "boot_entry.h"
#include "main.h"




#if BOOT_FUSE_BACKEND_SELECTED == BOOT_FUSE_BACKEND_SPI_FLASH
#include "boot_fuse_spiFlash.h"
#define ACTIVE_DRIVER &spi_flash_fuse_driver

#elif BOOT_FUSE_BACKEND_SELECTED == BOOT_FUSE_BACKEND_QSPI_FLASH
#include "boot_fuse_qspiFlash.h"
#define ACTIVE_DRIVER &qspi_flash_fuse_driver

#elif BOOT_FUSE_BACKEND_SELECTED == BOOT_FUSE_BACKEND_EEPROM
#include "boot_fuse_eeprom.h"
#define ACTIVE_DRIVER &eeprom_fuse_driver

#else
#error "Invalid BOOT_FUSE_BACKEND_SELECTED in boot_defs.h"
#endif




typedef void (*pFunction)(void);



pFunction Jump_To_Application;



void check_and_jump_to_bootloader() {
    
   fuse_mem_deviceInit(ACTIVE_DRIVER);
    fuse_set();
    
    uint8_t fuse_status = fuse_read();

    if (fuse_status == BOOT_FUSE_SET) {



    }
 
    else
    {
        if ((((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0xFF000000) == 0x20000000))
        {
            typedef void (*pFunction)(void);
            uint32_t JumpAddress = *(__IO uint32_t*)(APPLICATION_ADDRESS + 4);
            Jump_To_Application = (pFunction)JumpAddress;

            HAL_RCC_DeInit();
            HAL_DeInit();

            SysTick->CTRL = 0;
            SysTick->LOAD = 0;
            SysTick->VAL = 0;

            __disable_irq();
            SCB->VTOR = APPLICATION_ADDRESS;
            __set_MSP(*(__IO uint32_t*)APPLICATION_ADDRESS);
            Jump_To_Application();
        }
    }

        
}

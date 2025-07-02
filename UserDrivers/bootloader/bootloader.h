// bootloader.h
#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Called from app.c to set up backend hardware (QSPI/SPI, etc.)
void bootloader_driver_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __BOOTLOADER_H */


#ifndef __STAGING_QSPI_FLASH_H
#define __STAGING_QSPI_FLASH_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "boot_defs.h"
#include "boot_staging.h"

#ifdef __cplusplus
extern "C" {
#endif


extern const BootStagingBackend staging_qspiFlash_backend;

#define QSPI_STAGE_VER      0

#ifdef __cplusplus
}
#endif

#endif /* __STAGING_QSPI_FLASH_H */

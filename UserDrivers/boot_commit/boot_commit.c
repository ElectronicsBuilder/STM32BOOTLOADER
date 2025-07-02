#include "boot_commit.h"
#include "boot_defs.h"



#if BOOT_STAGING_BACKEND_SELECTED == BOOT_STAGING_BACKEND_QSPI_FLASH
#include "commit_qspi_flash.h"
static const BootCommitBackend* active_commit_backend = &commit_qspi_backend;

#elif BOOT_STAGING_BACKEND_SELECTED == BOOT_STAGING_BACKEND_SPI_FLASH
#include "commit_spi_flash.h"
static const BootCommitBackend* active_commit_backend = &commit_spi_backend;

#elif BOOT_STAGING_BACKEND_SELECTED == BOOT_STAGING_BACKEND_INTERNAL_FLASH
#include "commit_internal_flash.h"     // Include the new internal backend
static const BootCommitBackend* active_commit_backend = &commit_internal_backend;

#else
#error "No valid BOOT_STAGING_BACKEND_SELECTED for commit"
#endif


bool boot_commit_from_staging(const staging_binary_info_t *info) {
    return active_commit_backend && active_commit_backend->commit(info);
}

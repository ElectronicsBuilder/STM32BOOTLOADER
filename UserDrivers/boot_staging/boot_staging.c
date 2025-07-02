#include "boot_staging.h"
#include "boot_defs.h"

  


#if BOOT_STAGING_BACKEND_SELECTED == BOOT_STAGING_BACKEND_QSPI_FLASH
#include "staging_qspi_flash.h"
static const BootStagingBackend* active_backend = &staging_qspiFlash_backend;

#elif BOOT_STAGING_BACKEND_SELECTED == BOOT_STAGING_BACKEND_SPI_FLASH
#include "staging_spi.h"
static const BootStagingBackend* active_backend = &staging_spi_backend;

#elif BOOT_STAGING_BACKEND_SELECTED == BOOT_STAGING_BACKEND_INTERNAL_FLASH
#include "staging_internal_flash.h"     // Include the new internal backend
static const BootStagingBackend* active_backend = &staging_internalFlash_backend;

#else
#error "No valid BOOT_STAGING_BACKEND_SELECTED"
#endif

bool staging_init(void) {
    return active_backend && active_backend->init();
}

bool staging_prepare(const staging_binary_info_t *info)
{

    return active_backend && active_backend->prepare(info);
}
bool staging_write_chunk(uint32_t offset, const uint8_t *data, size_t len) {
    return active_backend && active_backend->write(offset, data, len);
}

bool staging_finalize(const staging_binary_info_t *info) {
    return active_backend && active_backend->finalize(info);
}

bool staging_verify(void) {
    return active_backend && active_backend->verify();
}

bool staging_mark_valid(void) {
    return active_backend && active_backend->mark_valid();
}

bool staging_read_metadata(uint32_t *size_out, uint32_t *crc_out, uint32_t *flags_out) {
    return active_backend && active_backend->read_metadata(size_out, crc_out, flags_out);
}

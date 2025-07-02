#include "extmem_writer.h"
#include "boot_defs.h"


#if EXTMEM_WRITER_BACKEND_SELECTED == EXTMEM_WRITER_BACKEND_QSPI_FLASH
#include "extmem_writer_qspi_flash.h"
static const BootExtmemWriter* active_extmem_writer = &extmem_writer_qspi_flash;

#elif EXTMEM_WRITER_BACKEND_SELECTED == EXTMEM_WRITER_BACKEND_FLASH
#include "extmem_writer_flash.h"
static const BootStagingBackend* active_backend = &extmem_writer_flash;

#else
#error "No valid BOOT_STAGING_BACKEND_SELECTED"
#endif

bool extmem_writer_init(void) {
    return active_extmem_writer && active_extmem_writer->init();
}

bool extmem_writer_prepare(const extmem_info_t *info) {
    return active_extmem_writer && active_extmem_writer->prepare(info);
}

bool extmem_writer_write(uint32_t offset, const uint8_t *data, size_t len) {
    return active_extmem_writer && active_extmem_writer->write(offset, data, len);
}

bool extmem_writer_finalize(const extmem_info_t *info) {
    return active_extmem_writer && active_extmem_writer->finalize(info);
}

bool extmem_writer_verify(void) {
    return active_extmem_writer && active_extmem_writer->verify();
}

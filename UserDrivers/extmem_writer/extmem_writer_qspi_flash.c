#include "extmem_writer_qspi_flash.h"
#include "extmem_writer.h"
#include "qspi_flash.h"
#include "log.h"
#include "boot_defs.h"
#include "boot_status.h"
#include <string.h>

extern QspiFlash qspiFlash;
extern bootloader_status_t g_boot_status;

static extmem_info_t extmem_meta;

static bool extmem_writer_init_qspi(void) {
    LOG_INFO("[EXTMEM] QSPI writer init");
    return true;
}

static bool extmem_writer_prepare_qspi(const extmem_info_t *info) {
    if (!info || info->size == 0 || info->size > QSPI_GUI_SIZE_MAX) return false;

    QFlashDeviceInfo qflashinfo = qspi_flash_get_info();
    const uint32_t sector_size = qflashinfo.sector_size;

    memcpy(&extmem_meta, info, sizeof(extmem_info_t));
    LOG_INFO("[EXTMEM] Prepare size=%lu, crc=0x%04X", info->size, info->crc);

    const uint32_t erase_end = QSPI_GUI_ADDR + ((info->size + sector_size - 1) / sector_size) * sector_size;

    for (uint32_t addr = QSPI_GUI_ADDR; addr < erase_end; addr += sector_size) {
        if (!qspi_flash_erase_sector(&qspiFlash, addr)) {
            LOG_ERROR("[EXTMEM] Failed to erase at 0x%08X", addr);
            return false;
        }
         LOG_INFO("[EXTMEM] Erased sector at 0x%08lX", addr);
    }
    return true;
}

static bool extmem_writer_write_qspi(uint32_t offset, const uint8_t *data, size_t len) {
    if (!data || len == 0 || offset + len > QSPI_GUI_SIZE_MAX) return false;

    g_boot_status.extmem_offset = offset;
    return qspi_flash_write_quad_dma_split(&qspiFlash, QSPI_GUI_ADDR + offset, data, len);
}

static bool extmem_writer_finalize_qspi(const extmem_info_t *info) {
    LOG_INFO("[EXTMEM] Finalize skipped (no metadata storage)");
    return true;
}

static bool extmem_writer_verify_qspi(void) {
    if (!qspi_flash_auto_poll_ready(&qspiFlash, 5000)) {
        LOG_ERROR("[EXTMEM] Flash not ready before verify");
        return false;
    }

    if (extmem_meta.size == 0 || extmem_meta.size > QSPI_GUI_SIZE_MAX) {
        LOG_ERROR("[EXTMEM] Invalid size in metadata");
        return false;
    }

    uint32_t remaining = extmem_meta.size;
    uint32_t offset = 0;
    uint8_t buf[256];
    uint16_t crc = 0xFFFF;

    while (remaining > 0) {
        uint32_t chunk = (remaining > sizeof(buf)) ? sizeof(buf) : remaining;
        if (!qspi_flash_read_quad_dma(&qspiFlash, QSPI_GUI_ADDR + offset, buf, chunk)) {
            LOG_ERROR("[EXTMEM] Read failed at offset 0x%08X", QSPI_GUI_ADDR + offset);
            return false;
        }
        HAL_Delay(1);
        crc = compute_crc16(crc, buf, chunk);
        offset += chunk;
        remaining -= chunk;
    }

    LOG_INFO("[EXTMEM] Verify complete: expected=0x%04X, actual=0x%04X", extmem_meta.crc, crc);
    return (crc == extmem_meta.crc);
}


const BootExtmemWriter extmem_writer_qspi_flash = {
    .init     = extmem_writer_init_qspi,
    .prepare  = extmem_writer_prepare_qspi,
    .write    = extmem_writer_write_qspi,
    .finalize = extmem_writer_finalize_qspi,
    .verify   = extmem_writer_verify_qspi
};

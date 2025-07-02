#include "staging_qspi_flash.h"
#include "qspi_flash.h"
#include "boot_defs.h"
#include "boot_status.h"
#include <string.h>

#include "log.h"
extern QspiFlash qspiFlash;
extern staging_binary_info_t stage_info;
extern bootloader_status_t g_boot_status;

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t size;
    uint32_t crc;
} StagingMetadata;

#define STAGING_MAGIC_VALID 0xA5A55A5A



static uint32_t staging_offset = 0;
static StagingMetadata current_meta;

static bool qspi_staging_init(void) {

    LOG_INFO("QSPI staging init...");
    boot_status_update(&g_boot_status, BOOT_PHASE_IDLE, 0, "STAGING INIT");

    return true;
}

static bool qspi_staging_write(uint32_t offset, const uint8_t *data, size_t len)
{
    g_boot_status.staged_offset = offset;

    if (!data || len == 0) return false;
    if ((offset + len) > QSPI_STAGING_SIZE) return false;

    uint32_t addr = QSPI_STAGING_ADDR + offset;

    if (!qspi_flash_write_quad_dma_split(&qspiFlash, addr, data, len)) {
        return false;
    }

#if QSPI_STAGE_VER
    // Read-back verification
    uint8_t verify_buf[1024];
    size_t check_len = (len < sizeof(verify_buf)) ? len : sizeof(verify_buf);
    HAL_Delay(1);
    qspi_flash_read_quad_dma(&qspiFlash, addr, verify_buf, sizeof(verify_buf));

    uint16_t computed_crc = compute_crc16(0xFFFF, verify_buf, len);
    LOG_INFO("Staging calculated CRC 0x%04X", computed_crc);

    if (memcmp(data, verify_buf, check_len) != 0) {
        LOG_ERROR("[QSPI] Mismatch on read-back verify at offset 0x%08lX", offset);
        return false;
    }

    LOG_INFO("QSPI BIN write/readback matched");
    LOG_INFO("QSPI BIN write at offset 0x%08lX", offset);
#endif

    // Update progress
    static uint16_t last_progress = 0;
    uint32_t total_written = offset + len;
    uint16_t progress = (uint16_t)((total_written * 100UL) / stage_info.size);

    // Only update every 10% or on final write
    if ((progress / 10 != last_progress / 10) || (progress == 100)) {
        last_progress = progress;
        boot_status_update(&g_boot_status, BOOT_PHASE_STAGING_WRITING, progress, "STAGING WRITE");
    }

    return true;
}

static bool qspi_staging_finalize(const staging_binary_info_t *info) {

    StagingMetadata meta = {
        .magic = STAGING_MAGIC_VALID,
        .size = info->size,
        .crc = info->crc
    };

     if (!qspi_flash_write_enable(&qspiFlash)) return false;
     if (!qspi_flash_erase_sector(&qspiFlash, QSPI_METADATA_ADDR)) return false;

     if (!qspi_flash_write_quad_dma_split(&qspiFlash, QSPI_METADATA_ADDR, (uint8_t*)&meta, sizeof(meta))) return false;
     if (!qspi_flash_auto_poll_ready(&qspiFlash, 1000)) return false;

    LOG_INFO("QSPI Staging Finalize: size=%lu, crc=0x%04X", info->size, info->crc);

    boot_status_update(&g_boot_status, BOOT_PHASE_STAGING_WRITING, 0, "STAGE FINALIZE");

    return true;
}


static bool qspi_staging_verify(void)
{
    StagingMetadata meta;
    memset(&meta, 0, sizeof(meta));

    if (!qspi_flash_auto_poll_ready(&qspiFlash, 5000))
        return false;

    if (!qspi_flash_read_quad_dma(&qspiFlash, QSPI_METADATA_ADDR, (uint8_t*)&meta, sizeof(meta)))
        return false;

    HAL_Delay(10);

    if (meta.magic != STAGING_MAGIC_VALID || meta.size == 0 || meta.size > QSPI_STAGING_SIZE)
        return false;

    uint8_t verify_buf[256];
    uint32_t offset = 0;
    uint16_t crc = 0xFFFF;
    uint16_t last_progress = 0;

    while (offset < meta.size) {
        uint32_t chunk = (meta.size - offset > sizeof(verify_buf)) ? sizeof(verify_buf) : (meta.size - offset);

        if (!qspi_flash_read_quad_dma(&qspiFlash, QSPI_STAGING_ADDR + offset, verify_buf, chunk))
            return false;

        HAL_Delay(1);

        crc = compute_crc16(crc, verify_buf, chunk);
        offset += chunk;

        uint16_t progress = (uint16_t)((offset * 100UL) / meta.size);
        boot_status_update_if_step(progress, &last_progress, BOOT_PHASE_STAGING_VERIFYING, "STAGE VERIFYING");
    }

    LOG_INFO("QSPI Staging Verify: size=%lu, stored_crc=0x%04X, calc_crc=0x%04X", meta.size, meta.crc, crc);

    return (crc == meta.crc);
}



static bool qspi_staging_mark_valid(void) {
    StagingMetadata meta;
    memset(&meta, 0, sizeof(meta));

    if (!qspi_flash_read_quad_dma(&qspiFlash, QSPI_METADATA_ADDR, (uint8_t*)&meta, sizeof(meta)))
        return false;

    meta.magic = STAGING_MAGIC_VALID;

    if (!qspi_flash_erase_sector(&qspiFlash, QSPI_METADATA_ADDR))
        return false;

    return qspi_flash_write_quad_dma_split(&qspiFlash, QSPI_METADATA_ADDR, (uint8_t*)&meta, sizeof(meta));
}


static bool qspi_staging_read_metadata(uint32_t *size_out, uint32_t *crc_out, uint32_t *flags_out) {
    if (!size_out || !crc_out || !flags_out) return false;

    StagingMetadata meta;
    memset(&meta, 0, sizeof(meta));

    if (!qspi_flash_read_quad_dma(&qspiFlash, QSPI_METADATA_ADDR, (uint8_t*)&meta, sizeof(meta)))
        return false;

    *size_out = meta.size;
    *crc_out = meta.crc;
    *flags_out = meta.magic;

    return true;
}

bool boot_staging_prepare(const staging_binary_info_t *info)
{
    uint32_t i = 0;
    uint16_t last_progress = 0;

    boot_status_update(&g_boot_status, BOOT_PHASE_STAGING_PREPARE, 0, "STAGING PREPARE");

    if (info->size== 0 || info->size > QSPI_STAGING_SIZE) {
        LOG_ERROR("Invalid staging size: %lu", info->size);
        return false;
    }

    memset(&current_meta, 0, sizeof(current_meta));
    staging_offset = 0;

    current_meta.magic = STAGING_MAGIC_VALID;
    current_meta.size  = info->size;
    current_meta.crc   = info->crc;

     QFlashDeviceInfo qflashinfo = qspi_flash_get_info();
    const uint32_t sector_size = qflashinfo.sector_size;

    const uint32_t erase_end = QSPI_STAGING_ADDR + ((info->size + sector_size - 1) / sector_size) * sector_size;
    const uint32_t num_sectors = (erase_end - QSPI_STAGING_ADDR) / sector_size;

    LOG_INFO("Staging prepare: size=%lu, crc=0x%04X",  info->size, info->crc);
    LOG_INFO("QSPI erase: %lu sectors (sector_size=%lu)", num_sectors, sector_size);


    for (uint32_t addr = QSPI_STAGING_ADDR; addr < erase_end; addr += sector_size, ++i) {
        if (!qspi_flash_erase_sector(&qspiFlash, addr)) {
            LOG_ERROR("[QSPI] Failed to erase sector at 0x%08lX", addr);
            return false;
        }

        LOG_INFO("[QSPI] Erased sector at 0x%08lX", addr);

        uint16_t progress = (uint16_t)(((i + 1) * 100UL) / num_sectors);
        boot_status_update_if_step(progress, &last_progress, BOOT_PHASE_STAGING_PREPARE, "STAGING PREP");
    }

    return true;
}




const BootStagingBackend staging_qspiFlash_backend = {
    .init = qspi_staging_init,
    .prepare = boot_staging_prepare,
    .write = qspi_staging_write,
    .finalize = qspi_staging_finalize,
    .verify = qspi_staging_verify,
    .mark_valid = qspi_staging_mark_valid,
    .read_metadata = qspi_staging_read_metadata
};



#include "staging_internal_flash.h"
#include "commit_internal_flash.h"
#include "boot_status.h"
#include "log.h"
#include <string.h>


extern bootloader_status_t g_boot_status;

static staging_binary_info_t internal_info;


static bool internal_staging_init(void) {

 LOG_INFO("Internal staging init...");

    return true;
}

static bool internal_prepare(const staging_binary_info_t *info) {
    if (!info || info->size == 0) return false;

    memcpy(&internal_info, info, sizeof(staging_binary_info_t));
    g_boot_status.staged_size = info->size;
    g_boot_status.staged_crc = info->crc;
    g_boot_status.chunk_total = (info->size + BOOT_BINARY_CHUNK_SIZE - 1) / BOOT_BINARY_CHUNK_SIZE;
    g_boot_status.phase = BOOT_PHASE_STAGING;
    strcpy(g_boot_status.last_status, "Staging started");

    return commit_internal_init(info);
}

static bool internal_write(uint32_t offset, const uint8_t *data, size_t len) {
    g_boot_status.staged_offset = offset;
    return commit_internal_write_chunk(offset, data, len);
}

static bool internal_finalize(const staging_binary_info_t *info) {
    return commit_internal_finalize();
}

static bool internal_verify(void) {
    return true;  // Verification done in finalize (CRC)
}

static bool internal_mark_valid(void) {
    return true;  // Optional future logic
}

static bool internal_read_metadata(uint32_t *size_out, uint32_t *crc_out, uint32_t *flags_out) {
    if (!size_out || !crc_out || !flags_out) return false;
    *size_out = internal_info.size;
    *crc_out  = internal_info.crc;
    *flags_out = 0;
    return true;
}

const BootStagingBackend staging_internalFlash_backend = {
    .init = internal_staging_init,
    .prepare = internal_prepare,
    .write = internal_write,
    .finalize = internal_finalize,
    .verify = internal_verify,
    .mark_valid = internal_mark_valid,
    .read_metadata = internal_read_metadata
};

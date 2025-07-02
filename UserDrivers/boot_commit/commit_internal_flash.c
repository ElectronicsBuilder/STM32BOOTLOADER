#include "commit_internal_flash.h"
#include "boot_defs.h"
#include "boot_status.h"
#include "log.h"
#include "boot_reply.h"
#include "boot_fuse.h"
#include <string.h>



static uint32_t commit_total_size = 0;
static uint16_t commit_expected_crc = 0;
static uint32_t commit_written = 0;

bool commit_internal_init(const staging_binary_info_t *info)
{
    if (info->size == 0 || info->size > (INTERNAL_FLASH_END - INTERNAL_FLASH_START)) {
        LOG_ERROR("[INTERNAL] Invalid image size");
        return false;
    }

    g_boot_status.commit_success = false;
    g_boot_status.commit_in_progress = true;
    g_boot_status.commit_total_size = info->size;
    g_boot_status.commit_current_offset = 0;
    strcpy(g_boot_status.commit_error, "");

    commit_total_size = info->size;
    commit_expected_crc = info->crc;
    commit_written = 0;

    HAL_FLASH_Unlock();

    // Erase internal flash sectors starting at APPLICATION_ADDRESS
    LOG_INFO("[COMMIT][QSPI] Erasing internal flash...");
    uint32_t remaining = commit_total_size;
    FLASH_EraseInitTypeDef erase;
    erase.TypeErase     = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    erase.Sector        = FLASH_START_SECTOR;
    erase.NbSectors     = 1;

    while (remaining > 0 && erase.Sector <= FLASH_END_SECTOR) {
        uint32_t sector_err = 0;
        if (HAL_FLASHEx_Erase(&erase, &sector_err) != HAL_OK) {
            LOG_ERROR("[COMMIT][QSPI] Erase failed at sector %lu", erase.Sector);
            strcpy(g_boot_status.commit_error, "ERASE_FAIL");
            g_boot_status.commit_in_progress = false;
            HAL_FLASH_Lock();
            return false;
        }
        erase.Sector++;
        remaining -= FLASH_SECTOR_SIZE;  // 128 KB per sector
    }

    // Confirm erase success
    if (*(volatile uint32_t*)APPLICATION_ADDRESS != 0xFFFFFFFF) {
        LOG_ERROR("[COMMIT][QSPI] Flash not erased properly at 0x08080000");
        strcpy(g_boot_status.commit_error, "ERASE_INCOMPLETE");
        g_boot_status.commit_in_progress = false;
        HAL_FLASH_Lock();
        return false;
    }

    return true;
}


bool commit_internal_write_chunk(uint32_t offset, const uint8_t* data, uint32_t len)
{
  if (!data || len == 0)
  {
    return false;
  }
  

    for (uint32_t programcounter = 0; programcounter < len; programcounter += 4) {
        uint32_t word = 0xFFFFFFFF;
        uint32_t chunk_len = (len - programcounter >= 4) ? 4 : (len - programcounter);
        memcpy(&word, data + programcounter, chunk_len);

        uint32_t dest_addr = APPLICATION_ADDRESS + offset + programcounter;

        if (word == 0xFFFFFFFF) {
            continue; // Skip blank word
        }

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dest_addr, word) != HAL_OK) {
            LOG_ERROR("[INTERNAL] Flash write failed at 0x%08lX, word=0x%08lX", dest_addr, word);
            LOG_ERROR("Flash Error Code: 0x%08lX", HAL_FLASH_GetError());
            return false;
        }
    }

    commit_written = offset + len;
    g_boot_status.commit_current_offset = commit_written;
    return true;
}


bool commit_internal_finalize(void)
{
   
    HAL_FLASH_Lock();

    LOG_INFO("[INTERNAL] Verifying CRC...");
    uint16_t calc_crc = compute_crc16(0xFFFF, ( uint8_t*)INTERNAL_FLASH_START, commit_total_size);
    if (calc_crc != commit_expected_crc) {
        LOG_ERROR("[INTERNAL] CRC mismatch: expected 0x%04X, got 0x%04X", commit_expected_crc, calc_crc);
        strcpy(g_boot_status.commit_error, "CRC_FAIL");
        g_boot_status.commit_in_progress = false;
        return false;
    }

    g_boot_status.commit_success = true;
    g_boot_status.commit_in_progress = false;
    g_boot_status.commit_current_offset = commit_total_size;
    LOG_INFO("[INTERNAL] Commit successful");

    return true;
}

const BootCommitBackend commit_internal_backend = {
    .commit = commit_internal_init
};

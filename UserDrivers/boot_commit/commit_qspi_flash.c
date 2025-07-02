#include "commit_qspi_flash.h"
#include "qspi_flash.h"
#include "boot_defs.h"
#include "boot_status.h"
#include "log.h"
#include "stm32f7xx_hal.h"
#include <string.h>



extern QspiFlash qspiFlash;
extern bootloader_status_t g_boot_status;

static bool qspi_commit_to_flash(const staging_binary_info_t *info)
{
    g_boot_status.phase = BOOT_PHASE_COMMITTING;
    uint16_t last_commit_progress = 0;
    uint16_t last_verify_progress = 0;

    uint16_t last_erased_progress = 0;
    uint32_t erased_sectors = 0;

    const uint32_t total_sectors = FLASH_END_SECTOR - FLASH_START_SECTOR + 1;
    if (total_sectors == 0) return false;

    if (!info || info->size == 0 || info->size > (APPLICATION_END_ADDRESS - APPLICATION_ADDRESS)) {
        LOG_ERROR("[COMMIT][QSPI] Invalid binary size: %lu", info ? info->size : 0);
        strcpy(g_boot_status.commit_error, "INVALID_SIZE");
        g_boot_status.commit_in_progress = false;
        return false;
    }

    g_boot_status.commit_current_offset = 0;
    g_boot_status.commit_total_size     = info->size;
    g_boot_status.commit_success        = false;
    g_boot_status.commit_in_progress    = true;
    strcpy(g_boot_status.commit_error, "");

    HAL_FLASH_Unlock();

    // Erase internal flash sectors starting at APPLICATION_ADDRESS
    LOG_INFO("[COMMIT][QSPI] Erasing internal flash...");
    uint32_t remaining = info->size;
    FLASH_EraseInitTypeDef erase;
    erase.TypeErase     = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    erase.Sector        = FLASH_START_SECTOR;
    erase.NbSectors     = 1;


    boot_status_update(&g_boot_status, BOOT_PHASE_FLASH_ERASING, 0, "ERASING FLASH");

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
        remaining -= FLASH_SECTOR_SIZE;
        erased_sectors++;

        uint16_t progress = (uint16_t)(((uint64_t)erased_sectors * 100) / total_sectors);
        boot_status_update_if_step(progress, &last_erased_progress, BOOT_PHASE_FLASH_ERASING, "ERASING FLASH");
    }

    // Confirm erase success
    if (*(volatile uint32_t*)APPLICATION_ADDRESS != 0xFFFFFFFF) {
        LOG_ERROR("[COMMIT][QSPI] Flash not erased properly at 0x08080000");
        strcpy(g_boot_status.commit_error, "ERASE_INCOMPLETE");
        g_boot_status.commit_in_progress = false;
        HAL_FLASH_Lock();
        return false;
    }
    boot_status_update(&g_boot_status, BOOT_PHASE_FLASH_ERASING, 0, "FLASH ERASED");
    // Enable memory-mapped QSPI
    qspi_flash_enable_mmap(&qspiFlash);

    // Clean and invalidate D-cache before using memory-mapped QSPI
    SCB_CleanInvalidateDCache();

    LOG_INFO("[COMMIT][QSPI] Writing %lu bytes to internal flash...", info->size);

    for (uint32_t offset = 0; offset < info->size; offset += 4) {
        g_boot_status.commit_current_offset = offset;

        uint32_t data;
        memcpy(&data, (const void*)(QSPI_MEM_MAPPED_ADDR + offset), sizeof(uint32_t));

        if (data == 0xFFFFFFFF)
        {
            LOG_WARN("[COMMIT][QSPI] Skipping blank word at offset 0x%08lX", offset);
            continue;
        }

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APPLICATION_ADDRESS + offset, data) != HAL_OK) {
            LOG_ERROR("[COMMIT][QSPI] Write failed at 0x%08lX, data=0x%08lX", APPLICATION_ADDRESS + offset, data);
            strcpy(g_boot_status.commit_error, "WRITE_FAIL");
            g_boot_status.commit_in_progress = false;
            HAL_FLASH_Lock();
            return false;
        }
    // --- Write Progress ---

        uint16_t progress = (uint16_t)(((offset + 4) * 100UL) / info->size);
        boot_status_update_if_step(progress, &last_commit_progress, BOOT_PHASE_COMMITTING, "WRITING FLASH");
    }

    HAL_FLASH_Lock();

    uint8_t verify_buf[256];
    uint32_t offset = 0;
    uint16_t crc = 0xFFFF;

    while (offset < info->size) {
        uint32_t chunk = ((info->size - offset) > sizeof(verify_buf)) ? sizeof(verify_buf) : (info->size - offset);
        memcpy(verify_buf, (const void*)(APPLICATION_ADDRESS + offset), chunk);

        crc = compute_crc16(crc, verify_buf, chunk);
        offset += chunk;

          // --- Verify Progress ---

        uint16_t progress = (uint16_t)(((offset + 4) * 100UL) / info->size);
        boot_status_update_if_step(progress, &last_verify_progress, BOOT_PHASE_FLASH_VERIFYING, "VERIFYING FLASH");
    }


    if (crc != info->crc) {
    LOG_ERROR("[COMMIT][QSPI] CRC mismatch: expected 0x%04X, got 0x%04X", info->crc, crc);
    strcpy(g_boot_status.commit_error, "CRC_FAIL");
    g_boot_status.commit_in_progress = false;
    return false;
}

    g_boot_status.commit_success = true;
    g_boot_status.commit_in_progress = false;
    g_boot_status.commit_current_offset = info->size;

    LOG_INFO("[COMMIT][QSPI]  Commit successful");
    boot_status_update(&g_boot_status, BOOT_PHASE_FLASH_VERIFIED, 100, "FLASH VERIFY");
    HAL_Delay(1000);
    boot_status_update(&g_boot_status, BOOT_PHASE_FLASH_VERIFIED, 100, "REBOOTING");
    HAL_Delay(1000);
    return true;
}


const BootCommitBackend commit_qspi_backend = {
    .commit = qspi_commit_to_flash
};

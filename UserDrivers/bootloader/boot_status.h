#ifndef __BOOT_STATUS_H
#define __BOOT_STATUS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{

    boot_status_update_init = 0,
     boot_status_update_start,
    boot_status_update_done
}boot_status_update_t;


typedef enum {
    BOOT_PHASE_IDLE = 0,
    BOOT_PHASE_STAGING,
    BOOT_PHASE_STAGING_PREPARE,
    BOOT_PHASE_STAGING_WRITING,
    BOOT_PHASE_STAGING_VERIFYING,
    BOOT_PHASE_VERIFYING,
    BOOT_PHASE_COMMITTING,
    BOOT_PHASE_FLASH_ERASING,
    BOOT_PHASE_FLASH_WRITING,
    BOOT_PHASE_FLASH_VERIFYING,
    BOOT_PHASE_FLASH_VERIFIED,
    BOOT_PHASE_DONE,
    BOOT_PHASE_ERROR
} boot_phase_t;

typedef struct {
    uint32_t last_cmd;

    // Staging details
    uint32_t staged_size;
    uint16_t staged_crc;
    uint32_t chunk_written;
    uint32_t chunk_total;
    uint32_t staged_offset;
    // Flash write progress
    uint32_t flash_written_bytes;

    // External memory transfer
    uint32_t extmem_size;
    uint16_t extmem_crc;
    uint32_t extmem_offset;
    uint32_t extmem_chunk_written;
    uint32_t extmem_chunk_total;
    bool     extmem_verified;
    bool     extmem_active;



    // Commit progress
    uint32_t commit_current_offset;
    uint32_t commit_total_size;
    bool     commit_in_progress;
    bool     commit_success;
    char     commit_error[32];

    // State flags
    bool staging_verified;
    bool flash_written;

    // General phase and error info
    boot_phase_t phase;
    uint32_t last_error_code;
    char last_status[32];

    // Progress Bar 
    uint16_t last_status_progress;
    boot_status_update_t update_status ;
} bootloader_status_t;

extern bootloader_status_t g_boot_status;

void boot_status_reset(void);
void boot_status_log(void);  // Optional: for logging/debug print
void boot_status_update(bootloader_status_t *status, boot_phase_t phase, uint16_t progress, const char* msg);

static inline void boot_status_update_if_step(uint16_t progress, uint16_t* last_progress, boot_phase_t phase, const char* msg)
{
    if ((progress / 10 != *last_progress / 10) || (progress == 100)) {
        *last_progress = progress;
        boot_status_update(&g_boot_status, phase, progress, msg);
    }
}

#ifdef __cplusplus
}
#endif

#endif // __BOOT_STATUS_H

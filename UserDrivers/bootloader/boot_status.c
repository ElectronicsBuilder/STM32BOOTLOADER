#include "boot_status.h"
#include <string.h>
#include "log.h"  
#include "boot_defs.h"
bootloader_status_t g_boot_status;


void boot_status_reset(void)
{
    memset(&g_boot_status, 0, sizeof(g_boot_status));
    g_boot_status.phase = BOOT_PHASE_IDLE;
    strcpy(g_boot_status.last_status, "Idle");
}

void boot_status_log(void)
{
    LOG_INFO("[Status] Phase: %u, Written: %lu / %lu, Flash: %lu, Status: %s",
             g_boot_status.phase,
             g_boot_status.chunk_written,
             g_boot_status.chunk_total,
             g_boot_status.flash_written_bytes,
             g_boot_status.last_status);
}



void boot_status_update(bootloader_status_t *status, boot_phase_t phase, uint16_t progress, const char* msg)
{

    status->update_status = boot_status_update_start;
    strcpy(status->last_status, msg);
    status->last_status_progress = progress;
    status->phase = phase;
    while (status->update_status != boot_status_update_done) {
        #if BOOTLOADER_USE_RTOS
        osDelay(1);
        #elif
        HAL_Delay(1);
        #endif 
    }
}
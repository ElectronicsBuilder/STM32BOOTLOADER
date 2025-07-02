#include "binary_loader.h"
#include "log.h"
#include "uart.h"
#include "boot_defs.h"
#include "boot_staging.h"
#include "boot_reply.h"
#include "boot_status.h"

#include <string.h>

static uint32_t total_offset = 0;
volatile uint32_t computed_chunk_crc = 0;

extern uint32_t expected_binary_size; 
extern uint32_t expected_binary_crc;   

extern staging_binary_info_t stage_info;

extern bootloader_status_t g_boot_status;

void process_binary_chunk(const uint8_t *data, size_t len)
{
    if (len < 2) {
        LOG_ERROR("Chunk too small");
        boot_reply_send(BOOT_REPLY_CHUNK_FAIL);
        return;
    }

    uint16_t received_crc = (data[len - 2] << 8) | data[len - 1];
    uint16_t computed_crc = compute_crc16(0xFFFF, (uint8_t*)data, len - 2);

    if (received_crc != computed_crc) {
        LOG_ERROR("CRC mismatch: expected 0x%04X, got 0x%04X", computed_crc, received_crc);
        boot_reply_send(BOOT_REPLY_CRC_FAIL);
        return;
    }

    LOG_INFO("CRC 0x%04X valid. Passing chunk to stage...", computed_crc);
    g_boot_status.chunk_written++;

    if (!staging_write_chunk(total_offset, data, len - 2)) {
        LOG_ERROR("Staging write failed at offset 0x%08lX", total_offset);
        boot_reply_send(BOOT_REPLY_CHUNK_FAIL);
        return;
    }



    LOG_INFO("Sending CHUNK_OK: offset=%lu", total_offset);
    boot_reply_send_chunk_ack(BOOT_REPLY_CHUNK_OK, total_offset);
    total_offset += (len - 2);

    if (total_offset >= stage_info.size) {
        LOG_INFO("All chunks received, finalizing staging...");
        
        if (!staging_finalize(&stage_info)) {
            LOG_ERROR("Staging finalize failed");
            boot_reply_send(BOOT_REPLY_CHUNK_FAIL);
            return;
        }

       // g_boot_status.phase = BOOT_PHASE_VERIFYING;
        //strcpy(g_boot_status.last_status, "Verifying...");
        //strcpy(g_boot_status.last_status, "VERIFYING");

        #if (BOOT_STAGING_BACKEND_SELECTED != BOOT_STAGING_BACKEND_INTERNAL_FLASH)

             if (!staging_verify()) {                           // staging verification only for external Mems 
            LOG_ERROR("Staging verification failed");
            boot_reply_send(BOOT_REPLY_CRC_FAIL);
            return;               
        }
        #endif 

        g_boot_status.staging_verified = true;
        g_boot_status.phase = BOOT_PHASE_DONE;

        LOG_INFO("Staging verified successfully! (offset=%lu)", total_offset);
        boot_reply_send_chunk_ack(BOOT_REPLY_VERIFIED, total_offset);
       

        uart_set_mode(UART_MODE_BOOTLOADER_RX);  // <-- switch back to receive commands
        HAL_Delay(250);
        send_framed_response(RESP_TYPE_REPLY, "STAGING_OK");
        LOG_INFO(" Sent STAGING_OK");
      
    }
}

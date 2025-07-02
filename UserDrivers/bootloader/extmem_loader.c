// extmem_loader.c
#include "extmem_loader.h"
#include "extmem_writer.h"
#include "boot_defs.h"
#include "boot_reply.h"
#include "boot_status.h"
#include "uart.h"
#include "log.h"
#include <string.h>

static uint32_t extmem_offset = 0;
static extmem_info_t extmem_info;
extern bootloader_status_t g_boot_status;

void extmem_loader_reset(void) {
    extmem_offset = 0;
    memset(&extmem_info, 0, sizeof(extmem_info));
}

void extmem_loader_set_info(uint32_t size, uint32_t crc) {
    extmem_info.size = size;
    extmem_info.crc = crc;
    extmem_writer_prepare(&extmem_info);
}

void  process_external_memory_chunk(const uint8_t *data, size_t len) {
    if (len < 2) {
        LOG_ERROR("EXTMEM chunk too small");
        boot_reply_send(BOOT_REPLY_CHUNK_FAIL);
        return;
    }

    uint16_t received_crc = (data[len - 2] << 8) | data[len - 1];
    uint16_t computed_crc = compute_crc16(0xFFFF, (uint8_t*)data, len - 2);

    if (received_crc != computed_crc) {
        LOG_ERROR("EXTMEM CRC mismatch: expected 0x%04X, got 0x%04X", computed_crc, received_crc);
        boot_reply_send(BOOT_REPLY_CRC_FAIL);
        return;
    }

    LOG_INFO("[EXTMEM] CRC 0x%04X valid at offset 0x%08lX", computed_crc, extmem_offset);
    g_boot_status.extmem_chunk_written++;

    if (!extmem_writer_write(extmem_offset, data, len - 2)) {
        LOG_ERROR("[EXTMEM] Write failed at offset 0x%08lX", extmem_offset);
        boot_reply_send(BOOT_REPLY_CHUNK_FAIL);
        return;
    }

    boot_reply_send_chunk_ack(BOOT_REPLY_CHUNK_OK, extmem_offset);
    extmem_offset += (len - 2);

    if (extmem_offset >= extmem_info.size) {
        LOG_INFO("[EXTMEM] All data written, verifying...");

        if (!extmem_writer_verify()) {
            LOG_ERROR("[EXTMEM] CRC verify failed");
            boot_reply_send(BOOT_REPLY_CRC_FAIL);
            return;
        }

        g_boot_status.phase = BOOT_PHASE_DONE;
        g_boot_status.staging_verified = true;
        strcpy(g_boot_status.last_status, "EXTMEM verified");

        boot_reply_send_chunk_ack(BOOT_REPLY_VERIFIED, extmem_offset);
        uart_set_mode(UART_MODE_BOOTLOADER_RX);
        HAL_Delay(250);
        send_framed_response(RESP_TYPE_REPLY, "EXTMEM_OK");
    }
}

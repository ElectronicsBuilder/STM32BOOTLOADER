#include "command_parser.h"
#include "extmem_loader.h"
#include "log.h"
#include "boot_defs.h"
#include "boot_fuse.h"
#include "uart.h"
#include "boot_staging.h"
#include "boot_status.h"
#include "boot_commit.h"
#include "boot_reply.h"

#include <string.h>

     staging_binary_info_t stage_info;
     staging_binary_info_t stage_info;

     extern bootloader_status_t g_boot_status;

// Packet: [STX][LEN][CMD][ARGS...][CRC][ETX]
typedef enum {
    CMD_STATE_WAIT_START,
    CMD_STATE_LEN,
    CMD_STATE_DATA,
    CMD_STATE_WAIT_END
} CmdParserState;

static CmdParserState state = CMD_STATE_WAIT_START;
static uint8_t buffer[64];
static uint8_t cmd_index = 0;
static uint8_t expected_len = 0;

void process_packet_byte(uint8_t byte)
{
    switch (state) {
        case CMD_STATE_WAIT_START:
            if (byte == BOOT_CMD_STX) {
                cmd_index = 0;
                state = CMD_STATE_LEN;
            }
            break;

        case CMD_STATE_LEN:
            expected_len = byte;
            if (expected_len < 2 || expected_len >= sizeof(buffer)) {
                LOG_ERROR("Invalid length: %u", expected_len);
                state = CMD_STATE_WAIT_START;
                break;
            }
            buffer[cmd_index++] = byte; // save LEN
            state = CMD_STATE_DATA;
            break;

        case CMD_STATE_DATA:
            buffer[cmd_index++] = byte;
            if (cmd_index >= expected_len + 1) {  // +1 for LEN byte
                state = CMD_STATE_WAIT_END;
            }
            break;

        case CMD_STATE_WAIT_END:
            if (byte != BOOT_CMD_ETX) {
                LOG_WARN("Packet discarded (bad end byte)");
                state = CMD_STATE_WAIT_START;
                break;
            }

           

            uint8_t received_crc = buffer[expected_len]; // last byte of payload
            uint8_t computed_crc = boot_crc8(&buffer[1], expected_len - 1); // [CMD + ARGS]

            if (received_crc != computed_crc) {
                LOG_WARN("Invalid CRC: expected 0x%02X, got 0x%02X", computed_crc, received_crc);
            } else {
               // LOG_INFO("Packet received: %u bytes", expected_len);
                uint8_t cmd = buffer[1];
                const uint8_t* args = &buffer[2];
                uint8_t args_len = expected_len - 2;

                
                switch (cmd) {
                    case BOOT_CMD_GET_VERSION:
                        //uart_send_string("Bootloader v1.0\r\n");
                        send_framed_response(RESP_TYPE_REPLY, "Bootloader v1.0");
                        break;
                    case BOOT_CMD_JUMP_TO_APP:
                        //uart_send_string("Jumping to app\r\n");
                        send_framed_response(RESP_TYPE_REPLY, "Jumping to app");
                        // handle here. 
                            fuse_clear();
                        	NVIC_SystemReset();
                        break;
                    
                    case BOOT_CMD_BOOTSTART:
                        if (args_len < 6) {
                            send_framed_response(RESP_TYPE_ERROR, "TOO_SHORT");
                            break;
                        }

                        uint32_t bin_size = (args[0] << 24) | (args[1] << 16) |
                                            (args[2] << 8)  |  args[3];
                        uint16_t bin_crc = (args[4] << 8) | args[5];

                        LOG_INFO("[BOOTSTART] size=%lu, crc=0x%04X", bin_size, bin_crc);

                         stage_info.crc  =  bin_crc;
                         stage_info.size = bin_size;


                            boot_status_reset();
                            g_boot_status.last_cmd = BOOT_CMD_BOOTSTART;
                            g_boot_status.staged_size = stage_info.size;
                            g_boot_status.staged_crc =   stage_info.crc;
                            g_boot_status.chunk_total = ( stage_info.size + 1023) / 1024;
                            g_boot_status.phase = BOOT_PHASE_STAGING;
                            strcpy(g_boot_status.last_status, "Staging started");

                        if (staging_prepare(&stage_info)) {

                            send_framed_response(RESP_TYPE_REPLY, "OK");
                            HAL_Delay(100);  // Give UART time to flush out frame
                            uart_set_mode(UART_MODE_BOOTLOADER_DATA);
                        } else {
                            send_framed_response(RESP_TYPE_ERROR, "STAGE_FAIL");
                        }
                        break;
                     
                    
                    case BOOT_CMD_COMMIT:
                        LOG_INFO("[COMMIT] Request to finalize and commit staged firmware");

                        g_boot_status.last_cmd = BOOT_CMD_COMMIT;
                        strcpy(g_boot_status.last_status, "Committing...");
                        g_boot_status.phase = BOOT_PHASE_COMMITTING;


                        #if (BOOT_STAGING_BACKEND_SELECTED == BOOT_STAGING_BACKEND_INTERNAL_FLASH) 
                            
                            send_framed_response(RESP_TYPE_REPLY, "COMMIT_OK");
                            
                            fuse_clear();
                        	NVIC_SystemReset();
                        #else
                        
                        if (!g_boot_status.staging_verified) {
                            g_boot_status.phase = BOOT_PHASE_ERROR;
                            strcpy(g_boot_status.last_status, "Not verified");
                            send_framed_response(RESP_TYPE_ERROR, "NOT_VERIFIED");
                            break;
                        }
                            if (boot_commit_from_staging(&stage_info)) {
                           
                           
                            g_boot_status.phase = BOOT_PHASE_DONE;
                           // strcpy(g_boot_status.last_status, "Commit successful");
                            send_framed_response(RESP_TYPE_REPLY, "COMMIT_OK");
                            
                            fuse_clear();
                        	NVIC_SystemReset();
                        
                        } else {
                            g_boot_status.phase = BOOT_PHASE_ERROR;
                            strcpy(g_boot_status.last_status, "Commit failed");
                            send_framed_response(RESP_TYPE_ERROR, "COMMIT_FAIL");
                        }
                        #endif 


                        break;

                    case BOOT_CMD_EXTMEM_INIT:
                        if (args_len < 6) {
                            send_framed_response(RESP_TYPE_ERROR, "EXTMEM_ARG_ERR");
                            break;
                        }

                        uint32_t extmem_size = (args[0] << 24) | (args[1] << 16) |
                                            (args[2] << 8)  |  args[3];
                        uint16_t extmem_crc  = (args[4] << 8) | args[5];

                        LOG_INFO("[EXTMEM_INIT] size=%lu, crc=0x%04X", extmem_size, extmem_crc);

                        extmem_loader_reset();
                        extmem_loader_set_info(extmem_size, extmem_crc);

                        boot_status_reset();
                        g_boot_status.last_cmd = BOOT_CMD_EXTMEM_INIT;
                        g_boot_status.extmem_size = extmem_size;
                        g_boot_status.extmem_crc = extmem_crc;
                        g_boot_status.extmem_chunk_total = (extmem_size + 1023) / 1024;
                        g_boot_status.phase = BOOT_PHASE_STAGING;
                        strcpy(g_boot_status.last_status, "EXTMEM started");

                        send_framed_response(RESP_TYPE_REPLY, "EXTMEM_OK");
                        HAL_Delay(100);
                        uart_set_mode(UART_MODE_BOOTLOADER_EXT_MEM);
                        break;

                    default:
                        uart_send_string("Unknown command\r\n");
                        break;
                }
            }

            state = CMD_STATE_WAIT_START;
            break;
    }
}

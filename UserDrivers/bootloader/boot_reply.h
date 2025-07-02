#ifndef __BOOT_REPLY_H
#define __BOOT_REPLY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


#define BOOT_REPLY_CHUNK_OK    0x10
#define BOOT_REPLY_CHUNK_FAIL  0x11
#define BOOT_REPLY_CRC_FAIL    0x12
#define BOOT_REPLY_VERIFIED    0x20  

#define BOOT_REPLY_STAGING_OK  0x04 

typedef enum {
    RESP_TYPE_INFO  = 0x01,
    RESP_TYPE_WARN  = 0x02,
    RESP_TYPE_ERROR = 0x03,
    RESP_TYPE_REPLY = 0x04
} BootResponseType;

/**
 * @brief Sends a framed UART response with response type and message.
 * 
 * Frame format: [STX][LEN][TYPE][DATA...][CRC][ETX]
 * - STX: 0x7E same as BOOT_CMD_STX in boot_defs
 * - ETX: 0x7F same as BOOT_CMD_ETX in boot_defs    
 * - LEN: bytes after LEN up to ETX (TYPE + DATA + CRC)
 *
 * @param type BootResponseType enum (INFO, WARN, ERROR, REPLY)
 * @param msg  Null-terminated UTF-8 string (max 250 chars used)
 */
void send_framed_response(BootResponseType type, const char* msg);
void boot_reply_send(uint8_t code);
void boot_reply_send_chunk_ack(uint8_t code, uint32_t offset);


#ifdef __cplusplus
}
#endif

#endif /* __BOOT_REPLY_H */

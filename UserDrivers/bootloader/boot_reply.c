#include "uart.h"
#include "string.h"
#include "boot_reply.h"
#include "uart.h"

#define STX 0x7E
#define ETX 0x7F



void send_framed_response(BootResponseType type, const char* msg)
{
    size_t len = strlen(msg);
    if (len > 250) len = 250;

    uint8_t buf[260] = {0};
    buf[0] = STX;
    buf[1] = (uint8_t)(1 + len + 1); // TYPE + DATA + CRC
    buf[2] = type;
    memcpy(&buf[3], msg, len);
    buf[3 + len] = boot_crc8(&buf[2], 1 + len);  // CRC over TYPE+DATA
    buf[4 + len] = ETX;

    uart_send_bytes(buf, 5 + len);
}

void boot_reply_send(uint8_t code)
{
    uint8_t frame[5];

    frame[0] = STX;
    frame[1] = 0x02;          // LEN (1 byte code + 1 byte CRC)
    frame[2] = code;
    frame[3] = boot_crc8(&frame[2], 1);  // CRC over reply code only
    frame[4] = ETX;

    uart_send_bytes(frame, sizeof(frame));
}

void boot_reply_send_chunk_ack(uint8_t code, uint32_t offset)
{
    uint8_t frame[9];

    frame[0] = STX;
    frame[1] = 1 /*code*/ + 4 /*offset*/ + 1 /*crc*/;
    frame[2] = code;
    frame[3] = (offset >> 24) & 0xFF;
    frame[4] = (offset >> 16) & 0xFF;
    frame[5] = (offset >> 8)  & 0xFF;
    frame[6] = (offset)       & 0xFF;
    frame[7] = boot_crc8(&frame[2], 5);  // CRC over code + offset
    frame[8] = ETX;

    uart_send_bytes(frame, 9);
}

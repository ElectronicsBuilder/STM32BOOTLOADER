#ifndef __COMMAND_PARSER_H
#define __COMMAND_PARSER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STX 0x7E
#define ETX 0x7F


void process_packet_byte(uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* __COMMAND_PARSER_H */

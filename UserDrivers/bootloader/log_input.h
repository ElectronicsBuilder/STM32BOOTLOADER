#ifndef __LOG_INPUT_H
#define __LOG_INPUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void uart_handle_log_input(uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* __LOG_INPUT_H */

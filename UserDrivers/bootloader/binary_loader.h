#ifndef __BINARY_LOADER_H
#define __BINARY_LOADER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void process_binary_chunk(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* __BINARY_LOADER_H */

#ifndef __EXTMEM_LOADER_H
#define __EXTMEM_LOADER_H

#include <stdint.h>
#include <stddef.h>
#include "boot_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

void extmem_loader_reset(void);
void extmem_loader_set_info(uint32_t size, uint32_t crc);
void process_external_memory_chunk(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* __EXTMEM_LOADER_H */

#ifndef __BOOT_STAGING_H
#define __BOOT_STAGING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "boot_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool (*init)(void);
    bool (*prepare)(const staging_binary_info_t *info);
    bool (*write)(uint32_t offset, const uint8_t *data, size_t len);
    bool (*finalize)(const staging_binary_info_t *info);
    bool (*verify)(void);
    bool (*mark_valid)(void);
    bool (*read_metadata)(uint32_t *size_out, uint32_t *crc_out, uint32_t *flags_out);
} BootStagingBackend;


bool staging_init(void);
bool staging_prepare(const staging_binary_info_t *info);
bool staging_write_chunk(uint32_t offset, const uint8_t *data, size_t len);
bool staging_finalize(const staging_binary_info_t *info);
bool staging_verify(void);
bool staging_mark_valid(void);
bool staging_read_metadata(uint32_t *size_out, uint32_t *crc_out, uint32_t *flags_out);



#ifdef __cplusplus
}
#endif

#endif /* __BOOT_STAGING_H */

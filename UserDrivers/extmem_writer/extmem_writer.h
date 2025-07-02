#ifndef __EXTMEM_WRITER_H
#define __EXTMEM_WRITER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "boot_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct
    {
        bool (*init)(void);
        bool (*prepare)(const extmem_info_t *info);
        bool (*write)(uint32_t offset, const uint8_t *data, size_t len);
        bool (*finalize)(const extmem_info_t *info);
        bool (*verify)(void);
    } BootExtmemWriter;

    
bool extmem_writer_init(void);
bool extmem_writer_prepare(const extmem_info_t *info);
bool extmem_writer_write(uint32_t offset, const uint8_t *data, size_t len);
bool extmem_writer_finalize(const extmem_info_t *info);
bool extmem_writer_verify(void);

#ifdef __cplusplus
}
#endif

#endif // __EXTMEM_WRITER_H

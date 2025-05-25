#ifndef BOOT_FUSE_H
#define BOOT_FUSE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*init)(void);
    uint8_t (*read)(void);
    bool (*set)(void);
    bool (*clear)(void);
} BootFuseDriver;

void fuse_mem_deviceInit(const BootFuseDriver* driver);
uint8_t fuse_read(void);
bool fuse_set(void);
bool fuse_clear(void);


#define BOOT_FUSE_SET_BYTE1     0x55
#define BOOT_FUSE_SET_BYTE2     0xAA
#define BOOT_FUSE_SET_BYTE3     0x5A

#define BOOT_FUSE_CLEAR_BYTE1   0xFF
#define BOOT_FUSE_CLEAR_BYTE2   0xFF
#define BOOT_FUSE_CLEAR_BYTE3   0xFF

#define BOOT_FUSE_SET        1
#define BOOT_FUSE_CLEAR      0
#define BOOT_FUSE_SIZE       3



#ifdef __cplusplus
}
#endif

#endif

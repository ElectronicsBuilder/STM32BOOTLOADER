#ifndef __BOOT_TRANSPORT_H
#define __BOOT_TRANSPORT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*init)(void);
    bool (*poll)(void);
    int  (*read)(uint8_t *buf, size_t len);
    int  (*write)(const uint8_t *buf, size_t len);
    void (*flush)(void);
} BootTransportDriver;

bool boot_transport_init(void);
bool boot_transport_poll(void);
int  boot_transport_read(uint8_t *buf, size_t len);
int  boot_transport_write(const uint8_t *buf, size_t len);
void boot_transport_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* __BOOT_TRANSPORT_H */

#include "boot_fuse.h"
#include <stddef.h>



static const BootFuseDriver* active_driver = NULL;

void fuse_mem_deviceInit(const BootFuseDriver* driver)
{
    active_driver = driver;
    if (driver && driver->init) {
        driver->init();
    }
}

uint8_t fuse_read(void)
{
    return (active_driver && active_driver->read)
        ? active_driver->read()
        : BOOT_FUSE_CLEAR;
}

bool fuse_set(void)
{
    return (active_driver && active_driver->set)
        ? active_driver->set()
        : false;
}

bool fuse_clear(void)
{
    return (active_driver && active_driver->clear)
        ? active_driver->clear()
        : false;
}

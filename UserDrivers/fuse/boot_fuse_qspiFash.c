#include "boot_fuse_qspiFlash.h"
#include "boot_defs.h"
#include "boot_fuse.h"
#include "qspi_flash.h"
#include <string.h>

#define FUSE_QSPI_FLASH_ADDR  0x00FFF000U  // Top 4KB of 16MB QSPI

extern QSPI_HandleTypeDef hqspi;
static QspiFlash qspiFlash;



static const uint8_t boot_fuse_expected[BOOT_FUSE_SIZE] = {
    BOOT_FUSE_SET_BYTE1,
    BOOT_FUSE_SET_BYTE2,
    BOOT_FUSE_SET_BYTE3
};

static const uint8_t boot_fuse_cleared[BOOT_FUSE_SIZE] = {
    BOOT_FUSE_CLEAR_BYTE1,
    BOOT_FUSE_CLEAR_BYTE2,
    BOOT_FUSE_CLEAR_BYTE3
};

static void qspi_init(void)
{
    qspi_flash_init(&qspiFlash, &hqspi);  
}

static uint8_t qspi_read(void)
{
    boot_fuse_metadata_t meta;
    qspi_flash_read(&qspiFlash, FUSE_QSPI_FLASH_ADDR, (uint8_t *)&meta, sizeof(meta));

    if (meta.magic != BOOT_FUSE_MAGIC ||
        meta.version != BOOT_FUSE_VERSION ||
        meta.len != BOOT_FUSE_DATA_LEN)
        return BOOT_FUSE_CLEAR;

    uint8_t crc = boot_crc8((uint8_t *)&meta, offsetof(boot_fuse_metadata_t, crc8));
    if (crc != meta.crc8)
        return BOOT_FUSE_CLEAR;

    return (memcmp(meta.fuse_data, boot_fuse_expected, BOOT_FUSE_SIZE) == 0) ? BOOT_FUSE_SET : BOOT_FUSE_CLEAR;
}


static bool qspi_set(void)
{
    boot_fuse_metadata_t meta = {
        .magic = BOOT_FUSE_MAGIC,
        .version = BOOT_FUSE_VERSION,
        .len = BOOT_FUSE_DATA_LEN,
    };

    memcpy(meta.fuse_data, boot_fuse_expected, BOOT_FUSE_SIZE);
    meta.crc8 = boot_crc8((uint8_t *)&meta, offsetof(boot_fuse_metadata_t, crc8));

    qspi_flash_erase_sector(&qspiFlash, FUSE_QSPI_FLASH_ADDR);
    qspi_flash_write(&qspiFlash, FUSE_QSPI_FLASH_ADDR, (uint8_t *)&meta, sizeof(meta));

    boot_fuse_metadata_t verify;
    qspi_flash_read(&qspiFlash, FUSE_QSPI_FLASH_ADDR, (uint8_t *)&verify, sizeof(verify));

    return memcmp(&verify, &meta, sizeof(meta)) == 0;
}

static bool qspi_clear(void)
{
    boot_fuse_metadata_t meta = {
        .magic = BOOT_FUSE_MAGIC,
        .version = BOOT_FUSE_VERSION,
        .len = BOOT_FUSE_DATA_LEN,
    };

    memcpy(meta.fuse_data, boot_fuse_cleared, BOOT_FUSE_SIZE);
    meta.crc8 = boot_crc8((uint8_t *)&meta, offsetof(boot_fuse_metadata_t, crc8));

    qspi_flash_erase_sector(&qspiFlash, FUSE_QSPI_FLASH_ADDR);
    qspi_flash_write(&qspiFlash, FUSE_QSPI_FLASH_ADDR, (uint8_t *)&meta, sizeof(meta));

    boot_fuse_metadata_t verify;
    qspi_flash_read(&qspiFlash, FUSE_QSPI_FLASH_ADDR, (uint8_t *)&verify, sizeof(verify));

    return memcmp(&verify, &meta, sizeof(meta)) == 0;
}






const BootFuseDriver qspi_flash_fuse_driver = {
    .init  = qspi_init,
    .read  = qspi_read,
    .set   = qspi_set,
    .clear = qspi_clear
};

#include "boot_fuse_spiFlash.h"
#include "boot_defs.h"
#include "boot_fuse.h"
#include "spi_flash.h"
#include "main.h"
#include <string.h>



extern SPI_HandleTypeDef hspi1;

#define FUSE_SPI_FLASH_ADDR  0x0000F000  // ← Localized here

static SpiFlash spiFlash;

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

static void spi_init(void)
{
    spi_flash_init(&spiFlash, &hspi1, FLASH_CS_GPIO_Port, FLASH_CS_Pin);
}

static uint8_t spiFlash_read(void)
{
    boot_fuse_metadata_t meta;
    spi_flash_read_data(&spiFlash, FUSE_SPI_FLASH_ADDR, (uint8_t *)&meta, sizeof(meta));

    if (meta.magic != BOOT_FUSE_MAGIC ||
        meta.version != BOOT_FUSE_VERSION ||
        meta.len != BOOT_FUSE_DATA_LEN)
        return BOOT_FUSE_CLEAR;

    uint8_t crc = boot_crc8((uint8_t *)&meta, offsetof(boot_fuse_metadata_t, crc8));
    if (crc != meta.crc8)
        return BOOT_FUSE_CLEAR;

    return (memcmp(meta.fuse_data, boot_fuse_expected, BOOT_FUSE_SIZE) == 0) ? BOOT_FUSE_SET : BOOT_FUSE_CLEAR;
}


static bool spiFlash_set(void)
{
    boot_fuse_metadata_t meta = {
        .magic = BOOT_FUSE_MAGIC,
        .version = BOOT_FUSE_VERSION,
        .len = BOOT_FUSE_DATA_LEN
    };
    memcpy(meta.fuse_data, boot_fuse_expected, BOOT_FUSE_SIZE);
    meta.crc8 = boot_crc8((uint8_t *)&meta, offsetof(boot_fuse_metadata_t, crc8));

    spi_flash_erase_sector(&spiFlash, FUSE_SPI_FLASH_ADDR);
    spi_flash_write_data(&spiFlash, FUSE_SPI_FLASH_ADDR, (uint8_t *)&meta, sizeof(meta));

    boot_fuse_metadata_t verify;
    spi_flash_read_data(&spiFlash, FUSE_SPI_FLASH_ADDR, (uint8_t *)&verify, sizeof(verify));

    return memcmp(&verify, &meta, sizeof(meta)) == 0;
}

static bool spiFlash_clear(void)
{
    boot_fuse_metadata_t meta = {
        .magic = BOOT_FUSE_MAGIC,
        .version = BOOT_FUSE_VERSION,
        .len = BOOT_FUSE_DATA_LEN
    };
    memcpy(meta.fuse_data, boot_fuse_cleared, BOOT_FUSE_SIZE);
    meta.crc8 = boot_crc8((uint8_t *)&meta, offsetof(boot_fuse_metadata_t, crc8));

    spi_flash_erase_sector(&spiFlash, FUSE_SPI_FLASH_ADDR);
    spi_flash_write_data(&spiFlash, FUSE_SPI_FLASH_ADDR, (uint8_t *)&meta, sizeof(meta));

    boot_fuse_metadata_t verify;
    spi_flash_read_data(&spiFlash, FUSE_SPI_FLASH_ADDR, (uint8_t *)&verify, sizeof(verify));

    return memcmp(&verify, &meta, sizeof(meta)) == 0;
}

const BootFuseDriver spi_flash_fuse_driver = {
    .init  = spi_init,
    .read  = spiFlash_read,
    .set   = spiFlash_set,
    .clear = spiFlash_clear
};

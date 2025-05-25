#include "boot_fuse_eeprom.h"
#include "boot_defs.h"
#include "boot_fuse.h"
#include <string.h>
#include "main.h"

#if defined(STM32F4xx)
#include "stm32f4xx_hal.h"
#elif defined(STM32F7xx)
#include "stm32f7xx_hal.h"
#else
#error "Unsupported STM32 family"
#endif

#define FUSE_EEPROM_ADDR_BASE  100  // start of metadata block

#define EEPROM_WRITE_DELAY_MS  5  // conservative write delay

static I2C_HandleTypeDef* eeprom_i2c = NULL;
static uint16_t eeprom_dev_addr = 0xA0;



static const uint8_t fuse_expected[BOOT_FUSE_SIZE] = {
    BOOT_FUSE_SET_BYTE1,
    BOOT_FUSE_SET_BYTE2,
    BOOT_FUSE_SET_BYTE3
};

static const uint8_t fuse_cleared[BOOT_FUSE_SIZE] = {
    BOOT_FUSE_CLEAR_BYTE1,
    BOOT_FUSE_CLEAR_BYTE2,
    BOOT_FUSE_CLEAR_BYTE3
};

void eeprom_fuse_bind_i2c(I2C_HandleTypeDef* hi2c, uint16_t dev_addr)
{
    eeprom_i2c = hi2c;
    eeprom_dev_addr = dev_addr;
}

static void eeprom_init(void) {
    // No-op, assumes I2C is already initialized
}

static HAL_StatusTypeDef eeprom_write_byte(uint16_t addr, uint8_t data)
{
    return HAL_I2C_Mem_Write(eeprom_i2c, eeprom_dev_addr, addr, I2C_MEMADD_SIZE_16BIT, &data, 1, HAL_MAX_DELAY);
}

static HAL_StatusTypeDef eeprom_read_byte(uint16_t addr, uint8_t* data)
{
    return HAL_I2C_Mem_Read(eeprom_i2c, eeprom_dev_addr, addr, I2C_MEMADD_SIZE_16BIT, data, 1, HAL_MAX_DELAY);
}

static HAL_StatusTypeDef eeprom_write_block(uint16_t addr, const uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        HAL_StatusTypeDef status = eeprom_write_byte(addr + i, data[i]);
        if (status != HAL_OK) return status;
        HAL_Delay(EEPROM_WRITE_DELAY_MS);
    }
    return HAL_OK;
}

static HAL_StatusTypeDef eeprom_read_block(uint16_t addr, uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        HAL_StatusTypeDef status = eeprom_read_byte(addr + i, &data[i]);
        if (status != HAL_OK) return status;
        HAL_Delay(EEPROM_WRITE_DELAY_MS);
    }
    return HAL_OK;
}



static uint8_t eeprom_read(void)
{
    boot_fuse_metadata_t meta = {0};
    if (eeprom_read_block(FUSE_EEPROM_ADDR_BASE, (uint8_t *)&meta, sizeof(meta)) != HAL_OK)
        return BOOT_FUSE_CLEAR;

    if (meta.magic != BOOT_FUSE_MAGIC ||
        meta.version != BOOT_FUSE_VERSION ||
        meta.len != BOOT_FUSE_DATA_LEN)
        return BOOT_FUSE_CLEAR;

    uint8_t crc = boot_crc8((uint8_t *)&meta, offsetof(boot_fuse_metadata_t, crc8));
    if (crc != meta.crc8)
        return BOOT_FUSE_CLEAR;

    return (memcmp(meta.fuse_data, fuse_expected, BOOT_FUSE_SIZE) == 0) ? BOOT_FUSE_SET : BOOT_FUSE_CLEAR;
}


static bool eeprom_set(void)
{
    boot_fuse_metadata_t meta = {
        .magic = BOOT_FUSE_MAGIC,
        .version = BOOT_FUSE_VERSION,
        .len = BOOT_FUSE_DATA_LEN
    };

    memcpy(meta.fuse_data, fuse_expected, BOOT_FUSE_SIZE);
    meta.crc8 = boot_crc8((uint8_t *)&meta, offsetof(boot_fuse_metadata_t, crc8));

    if (eeprom_write_block(FUSE_EEPROM_ADDR_BASE, (uint8_t *)&meta, sizeof(meta)) != HAL_OK)
        return false;

    boot_fuse_metadata_t verify;
    if (eeprom_read_block(FUSE_EEPROM_ADDR_BASE, (uint8_t *)&verify, sizeof(verify)) != HAL_OK)
        return false;

    return memcmp(&verify, &meta, sizeof(meta)) == 0;
}


static bool eeprom_clear(void)
{
    boot_fuse_metadata_t meta = {
        .magic = BOOT_FUSE_MAGIC,
        .version = BOOT_FUSE_VERSION,
        .len = BOOT_FUSE_DATA_LEN
    };

    memcpy(meta.fuse_data, fuse_cleared, BOOT_FUSE_SIZE);
    meta.crc8 = boot_crc8((uint8_t *)&meta, offsetof(boot_fuse_metadata_t, crc8));

    if (eeprom_write_block(FUSE_EEPROM_ADDR_BASE, (uint8_t *)&meta, sizeof(meta)) != HAL_OK)
        return false;

    boot_fuse_metadata_t verify;
    if (eeprom_read_block(FUSE_EEPROM_ADDR_BASE, (uint8_t *)&verify, sizeof(verify)) != HAL_OK)
        return false;

    return memcmp(&verify, &meta, sizeof(meta)) == 0;
}


const BootFuseDriver eeprom_fuse_driver = {
    .init  = eeprom_init,
    .read  = eeprom_read,
    .set   = eeprom_set,
    .clear = eeprom_clear
};

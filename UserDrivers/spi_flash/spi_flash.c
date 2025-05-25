#include "spi_flash.h"

void spi_flash_init(SpiFlash* flash, SPI_HandleTypeDef* spiHandle, GPIO_TypeDef* csPort, uint16_t csPin)
{
    flash->spiHandle = spiHandle;
    flash->csPort = csPort;
    flash->csPin = csPin;
    spi_flash_deselect(flash);
}

void spi_flash_select(SpiFlash* flash)
{
    HAL_GPIO_WritePin(flash->csPort, flash->csPin, GPIO_PIN_RESET);
}

void spi_flash_deselect(SpiFlash* flash)
{
    HAL_GPIO_WritePin(flash->csPort, flash->csPin, GPIO_PIN_SET);
}

void spi_flash_send_command(SpiFlash* flash, uint8_t cmd)
{
    HAL_SPI_Transmit(flash->spiHandle, &cmd, 1, HAL_MAX_DELAY);
}

void spi_flash_write_enable(SpiFlash* flash)
{
    spi_flash_select(flash);
    spi_flash_send_command(flash, 0x06); // Write Enable
    spi_flash_deselect(flash);
}

void spi_flash_write_data(SpiFlash* flash, uint32_t address, const uint8_t* data, size_t length)
{
    spi_flash_write_enable(flash);
    spi_flash_select(flash);

    uint8_t cmd[4] = {
        0x02,
        (uint8_t)(address >> 16),
        (uint8_t)(address >> 8),
        (uint8_t)(address)
    };

    HAL_SPI_Transmit(flash->spiHandle, cmd, 4, HAL_MAX_DELAY);
    HAL_SPI_Transmit(flash->spiHandle, (uint8_t*)data, length, HAL_MAX_DELAY);

    spi_flash_deselect(flash);
    spi_flash_poll_busy(flash);
}

void spi_flash_read_data(SpiFlash* flash, uint32_t address, uint8_t* buffer, size_t length)
{
    spi_flash_select(flash);

    uint8_t cmd[4] = {
        0x03,
        (uint8_t)(address >> 16),
        (uint8_t)(address >> 8),
        (uint8_t)(address)
    };

    HAL_SPI_Transmit(flash->spiHandle, cmd, 4, HAL_MAX_DELAY);
    HAL_SPI_Receive(flash->spiHandle, buffer, length, HAL_MAX_DELAY);

    spi_flash_deselect(flash);
}

void spi_flash_erase_sector(SpiFlash* flash, uint32_t address)
{
    spi_flash_write_enable(flash);
    spi_flash_select(flash);

    uint8_t cmd[4] = {
        0x20,
        (uint8_t)(address >> 16),
        (uint8_t)(address >> 8),
        (uint8_t)(address)
    };

    HAL_SPI_Transmit(flash->spiHandle, cmd, 4, HAL_MAX_DELAY);
    spi_flash_deselect(flash);

    spi_flash_poll_busy(flash);
}

void spi_flash_poll_busy(SpiFlash* flash)
{
    uint8_t cmd = 0x05;
    uint8_t status = 0x01;

    do {
        spi_flash_select(flash);
        HAL_SPI_Transmit(flash->spiHandle, &cmd, 1, HAL_MAX_DELAY);
        HAL_SPI_Receive(flash->spiHandle, &status, 1, HAL_MAX_DELAY);
        spi_flash_deselect(flash);
    } while (status & 0x01);
}

uint32_t spi_flash_read_device_id(SpiFlash* flash)
{
    uint8_t id[3] = {0};

    spi_flash_select(flash);
    spi_flash_send_command(flash, 0x9F);
    HAL_SPI_Receive(flash->spiHandle, id, 3, HAL_MAX_DELAY);
    spi_flash_deselect(flash);

    return (id[0] << 16) | (id[1] << 8) | id[2];
}

FlashDeviceInfo spi_flash_get_device_info(void)
{
    FlashDeviceInfo info = {
        .name = "W25Q128JVEIQ",
        .size_mbit = 128,
        .page_size = 256,
        .sector_size = 4096,
        .quad_enabled = false
    };
    return info;
}

void spi_flash_reset(SpiFlash* flash)
{
    // Reserved for future use
}

#include "st7789.h"
#include "main.h"
#include "LCDManager.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "log.h"

extern __IO uint8_t isTransmittingData;
st7789_ATTRIBUTES st7789;
extern void LCDManager_TransferComplete();
uint16_t DISPLAY_DONE_UPDATE = 0;

#ifdef USE_SINGLE_BUFFER
extern uint16_t frameBuf[(170 * 320 * 2 + 3) / 4];
#endif

static const st7789_config_t *st_cfg = NULL;
const st7789_config_t tft_config;

void st7789_init(const st7789_config_t *cfg) {
    st_cfg = cfg;
}

void Display_init(const st7789_config_t *cfg) {
    st7789_init(cfg);
    st7789_BKL(GPIO_PIN_RESET); 
    HAL_Delay(100); 
    st7789_BKL(GPIO_PIN_SET);
    st7789_setDir(ST7789_SCAN_DIR_VERTICAL);
    ST7789_Driver_init();
}

void st7789_InitIO(void) {
    GPIO_InitTypeDef GPIO_InitStructure = {
        .Speed = GPIO_SPEED_FREQ_HIGH,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL
    };

    GPIO_InitStructure.Pin = st_cfg->reset_pin;
    HAL_GPIO_Init(st_cfg->reset_port, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = st_cfg->dc_pin;
    HAL_GPIO_Init(st_cfg->dc_port, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = st_cfg->cs_pin;
    HAL_GPIO_Init(st_cfg->cs_port, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = st_cfg->bkl_pin;
    HAL_GPIO_Init(st_cfg->bkl_port, &GPIO_InitStructure);
}

void st7789_RES(GPIO_PinState state) {
    HAL_GPIO_WritePin(st_cfg->reset_port, st_cfg->reset_pin, state);
}

void st7789_DC(GPIO_PinState state) {
    HAL_GPIO_WritePin(st_cfg->dc_port, st_cfg->dc_pin, state);
}

void st7789_CS(GPIO_PinState state) {
    HAL_GPIO_WritePin(st_cfg->cs_port, st_cfg->cs_pin, state);
}

void st7789_BKL(GPIO_PinState state) {
    HAL_GPIO_WritePin(st_cfg->bkl_port, st_cfg->bkl_pin, state);
}

uint8_t SPI_SendByte(uint8_t data) {
    HAL_SPI_Transmit(st_cfg->hspi, &data, 1, HAL_MAX_DELAY);
    return data;
}

uint16_t SPI_Send16(uint16_t data) {
    uint8_t bytes[2];
    bytes[0] = data >> 8;
    bytes[1] = data & 0xFF;
    HAL_SPI_Transmit(st_cfg->hspi, bytes, 2, HAL_MAX_DELAY);
    return data;
}

void command(uint8_t c) {
    st7789_DC(GPIO_PIN_RESET);
    st7789_CS(GPIO_PIN_RESET);
    SPI_SendByte(c);
    st7789_CS(GPIO_PIN_SET);
}

void data(uint8_t c) {
    st7789_DC(GPIO_PIN_SET);
    st7789_CS(GPIO_PIN_RESET);
    SPI_SendByte(c);
    st7789_CS(GPIO_PIN_SET);
}

void ST7789_WriteData(const st7789_config_t *cfg, uint8_t *buff, size_t buff_size) {
    st7789_CS(GPIO_PIN_RESET);
    st7789_DC(GPIO_PIN_SET);

    while (buff_size > 0) {
        uint16_t chunk_size = buff_size > 65535 ? 65535 : buff_size;
        HAL_SPI_Transmit(cfg->hspi, buff, chunk_size, HAL_MAX_DELAY);
        buff += chunk_size;
        buff_size -= chunk_size;
    }

    st7789_CS(GPIO_PIN_SET);
}

void ST7789_SetRotation(uint8_t m) {
    command(ST7789_MADCTL);
    switch (m) {
    case 0:
        data(ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB);
        break;
    case 1:
        data(ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
        break;
    case 2:
        data(ST7789_MADCTL_RGB);
        break;
    case 3:
        data(ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
        break;
    default:
        break;
    }
}

void st7789_SetAttributes(uint8_t Scan_dir) {
    st7789.SCAN_DIR = Scan_dir;
    uint8_t MemoryAccessReg = 0x00;

    if (Scan_dir == HORIZONTAL) {
        st7789.HEIGHT = LCD_WIDTH;
        st7789.WIDTH  = LCD_HEIGHT;
        MemoryAccessReg = 0x70;
    } else {
        st7789.HEIGHT = LCD_HEIGHT;
        st7789.WIDTH  = LCD_WIDTH;
        MemoryAccessReg = 0x00;
    }

    command(0x36);
    data(MemoryAccessReg);
}

static void st7789_Reset(void) {
    st7789_RES(GPIO_PIN_SET);
    HAL_Delay(100);
    st7789_RES(GPIO_PIN_RESET);
    HAL_Delay(100);
    st7789_RES(GPIO_PIN_SET);
    HAL_Delay(150);
}

void st7789_setDir(st7789_scan_dir_t Scan_dir) {
    st7789_Reset();
    HAL_Delay(100);
    st7789_SetAttributes(Scan_dir);
}

void ST7789_Driver_init(void) {
    command(ST7789_COLMOD);
    data(ST7789_COLOR_MODE_16bit);

    command(ST7789_PORCH_CTRL_CMD);
    {
        uint8_t data_buf[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
        ST7789_WriteData(&tft_config, data_buf, sizeof(data_buf));
    }

    command(ST7789_GATE_CTRL_CMD);      data(ST7789_GATE_CTRL_DATA);
    command(ST7789_VCOM_CMD);           data(ST7789_VCOM_DATA);
    command(ST7789_PWR_CTRL1_CMD);      data(ST7789_PWR1_DATA);
    command(ST7789_PWR_CTRL2_CMD);      data(ST7789_PWR2_DATA);
    command(ST7789_VRH_VDV_CTRL_CMD);   data(ST7789_VRH_VDV_DATA);
    command(ST7789_VDV_SETTING_CMD);    data(ST7789_VDV_SETTING_DATA);
    command(ST7789_FR_CTRL_CMD);        data(ST7789_FR_CTRL_DATA);
    command(ST7789_PWR_CTRL3_CMD);      data(ST7789_PWR3_DATA1); data(ST7789_PWR3_DATA2);

    command(ST7789_GAMMA_POS_CMD);
    {
        uint8_t gamma_pos[] = {
            0xF0, 0x00, 0x04, 0x04, 0x04,
            0x05, 0x29, 0x33, 0x3E, 0x38,
            0x12, 0x12, 0x28, 0x30
        };
        ST7789_WriteData(&tft_config, gamma_pos, sizeof(gamma_pos));
    }

    command(ST7789_GAMMA_NEG_CMD);
    {
        uint8_t gamma_neg[] = {
            0xF0, 0x07, 0x0A, 0x0D, 0x0B,
            0x07, 0x28, 0x33, 0x3E, 0x36,
            0x14, 0x14, 0x29, 0x32
        };
        ST7789_WriteData(&tft_config, gamma_neg, sizeof(gamma_neg));
    }

    command(ST7789_INVON);
    ST7789_SetRotation(ST7789_ROTATION);
    command(ST7789_SLPOUT);
    HAL_Delay(120);
    command(ST7789_DISPON);
    HAL_Delay(50);
}

void ST7789_SendRGB565Buffer(const uint16_t *data, uint32_t length, uint32_t chunk_size) {
    uint32_t sent = 0;
    while (sent < length) {
        uint32_t current_chunk = (length - sent) > chunk_size ? chunk_size : (length - sent);
        for (uint32_t i = 0; i < current_chunk; i++) {
            SPI_Send16(data[sent + i]);
        }
        sent += current_chunk;
    }
}

void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data) {
    DISPLAY_DONE_UPDATE = 0;
    if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT)) return;
    if ((x + w - 1) >= LCD_WIDTH) w = LCD_WIDTH - x;
    if ((y + h - 1) >= LCD_HEIGHT) h = LCD_HEIGHT - y;
    setAddrWindow(x, y, x + w - 1, y + h - 1);
    st7789_DC(GPIO_PIN_SET);
    st7789_CS(GPIO_PIN_RESET);
    ST7789_SendRGB565Buffer(data, w * h, ((w * h) / 1));
    st7789_CS(GPIO_PIN_SET);
    DISPLAY_DONE_UPDATE = 1;
}

void ST7789_FillScreen(uint16_t color) {
    FillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}

void FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT)) return;
    if ((x + w - 1) >= LCD_WIDTH) w = LCD_WIDTH - x;
    if ((y + h - 1) >= LCD_HEIGHT) h = LCD_HEIGHT - y;
    setAddrWindow(x, y, x + w - 1, y + h - 1);
    st7789_DC(GPIO_PIN_SET);
    st7789_CS(GPIO_PIN_RESET);
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    for (int i = 0; i < w * h; i++) {
        SPI_SendByte(hi);
        SPI_SendByte(lo);
    }
    st7789_CS(GPIO_PIN_SET);
}

void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    const uint16_t X_OFFSET = 35;
    x0 += X_OFFSET;
    x1 += X_OFFSET;
    command(ST7789_CASET);
    data(x0 >> 8); data(x0 & 0xFF);
    data(x1 >> 8); data(x1 & 0xFF);
    command(ST7789_PASET);
    data(y0 >> 8); data(y0 & 0xFF);
    data(y1 >> 8); data(y1 & 0xFF);
    command(ST7789_RAMWR);
}

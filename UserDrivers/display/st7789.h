#ifndef ST7789_H
#define ST7789_H

#include "stm32f7xx_hal.h"
#include <stdint.h>
#include <stddef.h>

#define LCD_HEIGHT 320
#define LCD_WIDTH  170

#define USE_DMA
#define HORIZONTAL 0
#define VERTICAL   1

#define ST7789_COLOR_MODE_16bit 0x55

#define WHITE       0xFFFF
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define GRAY        0x8430
#define BRED        0xF81F
#define GRED        0xFFE0
#define GBLUE       0x07FF
#define BROWN       0xBC40
#define BRRED       0xFC07
#define DARKBLUE    0x01CF
#define LIGHTBLUE   0x7D7C
#define GRAYBLUE    0x5458
#define LIGHTGREEN  0x841F
#define LGRAY       0xC618
#define LGRAYBLUE   0xA651
#define LBBLUE      0x2B12

// ST7789 Commands
#define ST7789_NOP               0x00
#define ST7789_SWRESET           0x01
#define ST7789_RDDID             0x04
#define ST7789_RDDST             0x09
#define ST7789_SLPIN             0x10
#define ST7789_SLPOUT            0x11
#define ST7789_PTLON             0x12
#define ST7789_NORON             0x13
#define ST7789_INVOFF            0x20
#define ST7789_INVON             0x21
#define ST7789_DISPOFF           0x28
#define ST7789_DISPON            0x29
#define ST7789_CASET             0x2A
#define ST7789_PASET             0x2B
#define ST7789_RAMWR             0x2C
#define ST7789_RAMRD             0x2E
#define ST7789_PTLAR             0x30
#define ST7789_MADCTL            0x36
#define ST7789_COLMOD            0x3A

// Memory Access Control Bits
#define ST7789_MADCTL_MY         0x80
#define ST7789_MADCTL_MX         0x40
#define ST7789_MADCTL_MV         0x20
#define ST7789_MADCTL_ML         0x10
#define ST7789_MADCTL_RGB        0x00

// Additional Configuration Commands
#define ST7789_PORCH_CTRL_CMD    0xB2
#define ST7789_GATE_CTRL_CMD     0xB7
#define ST7789_VCOM_CMD          0xBB
#define ST7789_PWR_CTRL1_CMD     0xC0
#define ST7789_PWR_CTRL2_CMD     0xC2
#define ST7789_VRH_VDV_CTRL_CMD  0xC3
#define ST7789_VDV_SETTING_CMD   0xC4
#define ST7789_FR_CTRL_CMD       0xC6
#define ST7789_PWR_CTRL3_CMD     0xD0
#define ST7789_GAMMA_POS_CMD     0xE0
#define ST7789_GAMMA_NEG_CMD     0xE1

// Corresponding Data Values
#define ST7789_GATE_CTRL_DATA      0x35
#define ST7789_VCOM_DATA           0x1A
#define ST7789_PWR1_DATA           0x2C
#define ST7789_PWR2_DATA           0x01
#define ST7789_VRH_VDV_DATA        0x0B
#define ST7789_VDV_SETTING_DATA    0x20
#define ST7789_FR_CTRL_DATA        0x0F
#define ST7789_PWR3_DATA1          0xA4
#define ST7789_PWR3_DATA2          0xA1

#define ST7789_ROTATION 2

// Types
typedef enum {
    ST7789_SCAN_DIR_HORIZONTAL,
    ST7789_SCAN_DIR_VERTICAL
} st7789_scan_dir_t;

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    GPIO_TypeDef *dc_port;
    uint16_t dc_pin;
    GPIO_TypeDef *reset_port;
    uint16_t reset_pin;
    GPIO_TypeDef *bkl_port;
    uint16_t bkl_pin;
    uint16_t width;
    uint16_t height;
    st7789_scan_dir_t scan_dir;
} st7789_config_t;

typedef struct {
    uint16_t WIDTH;
    uint16_t HEIGHT;
    uint16_t SCAN_DIR;
} st7789_ATTRIBUTES;

extern st7789_ATTRIBUTES st7789;

// Public Functions
void st7789_init(const st7789_config_t *cfg);
void st7789_InitIO(void);
void st7789_RES(GPIO_PinState state);
void st7789_DC(GPIO_PinState state);
void st7789_CS(GPIO_PinState state);
void st7789_BKL(GPIO_PinState state);
void command(uint8_t c);
void data(uint8_t c);
void ST7789_WriteData(const st7789_config_t *cfg, uint8_t *buff, size_t buff_size);
void ST7789_SetRotation(uint8_t m);
void st7789_SetAttributes(uint8_t Scan_dir);
void st7789_setDir(st7789_scan_dir_t Scan_dir);
void ST7789_Driver_init(void);
void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);
void ST7789_FillScreen(uint16_t color);
void FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void Display_init(const st7789_config_t *cfg);
void st7789_Clear(uint16_t Color);

#endif // ST7789_H

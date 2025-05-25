#include "boot_defs.h"
#include "main.h"

#if defined(STM32F4xx)
#include "stm32f4xx_hal.h"
#elif defined(STM32F7xx)
#include "stm32f7xx_hal.h"
#else
#error "Unsupported STM32 family"
#endif


uint8_t boot_crc8(const uint8_t *data, size_t len) {
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; ++j)
            crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
    }
    return crc; 
}

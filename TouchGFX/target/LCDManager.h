#ifndef LCDMANAGER_H
#define LCDMANAGER_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

#define USE_SINGLE_BUFFER

void LCDManager_SendFrameBufferBlockWithPosition(uint16_t* pixels, uint16_t x, uint16_t y, uint16_t w, uint16_t h);


#ifdef USE_PARTIAL_BUFFER
void touchgfxDisplayDriverTransmitBlock(const uint8_t* pixels, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
int  touchgfxDisplayDriverTransmitActive(void);
#endif

uint32_t LCDManager_IsTransmittingData(void);
void     LCDManager_TransferComplete(void);

#ifdef __cplusplus
}
#endif

#endif // LCDMANAGER_H

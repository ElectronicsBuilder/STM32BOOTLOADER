
#include "LCDManager.h"
#include "st7789.h"


extern void LCDManager_TransferComplete();


void DisplayDriver_TransferCompleteCallback();

static __IO uint8_t isTransmittingData = 0;

extern uint16_t DISPLAY_DONE_UPDATE;


uint32_t LCDManager_IsTransmittingData(void)
{
	return isTransmittingData;
}

#ifdef USE_SINGLE_BUFFER
void LCDManager_SendFrameBufferBlockWithPosition(uint16_t* pixels, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{

	isTransmittingData = 1;

	#if defined(STM32F7)
    SCB_CleanDCache_by_Addr((uint32_t *)pixels, w * h * 2);
	#endif

	ST7789_DrawImage(x, y, w, h, pixels);

	if (DISPLAY_DONE_UPDATE == 1) {
		isTransmittingData = 0;
	
		}

}

#endif

#ifdef USE_PARTIAL_BUFFER


void LCDManager_SendFrameBufferBlockWithPosition(uint8_t* pixels, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{

	isTransmittingData = 1;

	ST7789_DrawImage(x, y, w, h, (uint16_t *)pixels);

	isTransmittingData = 0;
		LCDManager_TransferComplete();

}


extern "C"
void touchgfxDisplayDriverTransmitBlock(const uint8_t* pixels, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	isTransmittingData = 1;

	ST7789_DrawImage(x, y, w, h, (uint8_t *)pixels);

	if(isTransmittingData == 0)
	{
		DisplayDriver_TransferCompleteCallback();
	}

}






extern "C"

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef * hspi1)
{

		 isTransmittingData = 0;

}



extern "C"
int touchgfxDisplayDriverTransmitActive()
{
	return LCDManager_IsTransmittingData();
}


#endif




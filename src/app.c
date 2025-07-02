
#include "app.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f767xx.h"
#include "main.h"

#include "log.h"
#include "uart.h"
#include "qspi_flash.h"
#include "boot_defs.h"

#include "queue.h"


#include "binary_loader.h"
#include "command_parser.h"
#include "boot_transport.h"
#include "boot_staging.h"
#include "bootloader.h"
#include "st7789.h"

#include "app_touchgfx.h"

void heartbeatTask(void *argument);
void UART_Task(void *argument);
void GUI_task(void *argument);
void DISPLAY_task(void *argument);
void boot_binary_task(void *argument);


extern const st7789_config_t tft_config;

extern QueueHandle_t uart_cmd_queue;

#ifdef __cplusplus
extern "C" {
#endif

void touchgfx_signalVSyncTimer(void);  // C-linkage for C code

#ifdef __cplusplus
}
#endif

extern volatile bool  qspi_dma_tx_done;
extern SPI_HandleTypeDef hspi6;



osThreadId_t heartbeatTask_TaskHandle;
const osThreadAttr_t heartbeatTask_attributes = {
	.name = "heartbeat TASK",
	.stack_size = 2048 * 1,
	.priority = (osPriority_t)osPriorityNormal

};

osThreadId_t UARTTask_TaskHandle;
const osThreadAttr_t UARTTask_attributes = {
	.name = "UART TASK",
	.stack_size = 2048 * 1,
	.priority = (osPriority_t)osPriorityNormal

};

osThreadId_t GUI_TaskHandle;
const osThreadAttr_t GUITask_attributes = {
	.name = "GUI TASK",
	.stack_size = 2048 * 1,
	.priority = (osPriority_t)osPriorityNormal

};

osThreadId_t DISPLAY_TaskHandle;
const osThreadAttr_t DISPLAYTask_attributes = {
	.name = "DISPLAY TASK",
	.stack_size = 1024 * 1,
	.priority = (osPriority_t)osPriorityNormal

};



bool TouchGFX_init = false;
 int tickCounter;
 int digitalHours;
 int digitalMinutes;
 int digitalSeconds;
 uint16_t digitalDays;

uint32_t g_tickCounter;
int g_digitalSeconds;




void bootloader_app(void)
{
    LOG_INFO("🔄Aidley Controller Bootloader Started!");


    bootloader_driver_init();
    osKernelInitialize();

    heartbeatTask_TaskHandle            = osThreadNew(heartbeatTask, NULL, &heartbeatTask_attributes);
    UARTTask_TaskHandle                 = osThreadNew(UART_Task, NULL, &UARTTask_attributes);
    
    GUI_TaskHandle                      = osThreadNew(GUI_task, NULL, &GUITask_attributes);
    DISPLAY_TaskHandle                  = osThreadNew(DISPLAY_task, NULL, &DISPLAYTask_attributes);


    LOG_INFO("About to start FreeRTOS kernel...");
    osKernelStart();
    LOG_ERROR("osKernelStart() returned unexpectedly!\r\n");

    while (1)
    {
    }
}





void heartbeatTask(void *argument)
{
    HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_SET); // Clear Error LED


    for (;;) {
  
        osDelay(100);  // Run every ~100 ms
        HAL_GPIO_TogglePin(LED_HB_GPIO_Port, LED_HB_Pin);  
    
    }
}

void UART_Task(void *argument)
{
   osDelay(2000);
   boot_transport_init();
   staging_init();

   
    for (;;) {
        osDelay(1);  
        boot_transport_poll();
    }
}




void DISPLAY_task(void *argument)
{
    uint32_t tick = 0;

     st7789_config_t tft_config = {
        .hspi       = &hspi6,
        .cs_port    = TFT_CS_GPIO_Port,
        .cs_pin     = TFT_CS_Pin,
        .dc_port    = TFT_DC_GPIO_Port,
        .dc_pin     = TFT_DC_Pin,
        .reset_port = TFT_RESET_GPIO_Port,
        .reset_pin  = TFT_RESET_Pin,
        .bkl_port   = BKL_PWM_GPIO_Port,
        .bkl_pin    = BKL_PWM_Pin,
        .width      = LCD_WIDTH,
        .height     = LCD_HEIGHT,
        .scan_dir   = ST7789_SCAN_DIR_VERTICAL
    };

   
    Display_init(&tft_config);        // Init Display

    do
    {
        /* code */
        osDelay(10); 
    } while (TouchGFX_init != true);
   
    for (;;) {
        
       tick++;
        if (tick >= 16) {
            touchgfx_signalVSyncTimer();
            tick = 0;
        }
        osDelay(1); 
    }
}

void GUI_task(void *argument)
{

        MX_TouchGFX_Init();
	    /* Call PreOsInit function */
		MX_TouchGFX_PreOSInit();
        TouchGFX_init = true;
		MX_TouchGFX_Process();
    
   
    for (;;) {
      

    }
}










#include <gui/screen_screen/screenView.hpp>

#include <stdio.h>
#include <string.h>
#include "boot_status.h"

extern int tickCounter;
extern int digitalHours;
extern int digitalMinutes;
extern int digitalSeconds;
extern uint16_t digitalDays;
uint8_t clock_start_flag = 0;

extern uint32_t g_tickCounter;
extern int g_digitalSeconds;
extern bool TouchGFX_init;
volatile bool boot_status_busy = false;

extern bootloader_status_t g_boot_status;

screenView::screenView()
{
    g_digitalSeconds = 0;   
}

void screenView::setupScreen()
{
    screenViewBase::setupScreen();

    if (clock_start_flag == 0) {
    		digitalHours = digitalClock.getCurrentHour();
    		digitalMinutes = digitalClock.getCurrentMinute();
    		digitalSeconds = digitalClock.getCurrentSecond();
    		digitalDays = 0;
    	}


    	clock_start_flag = 1;


    TouchGFX_init = true;
}

void screenView::tearDownScreen()
{
    screenViewBase::tearDownScreen();
}


void screenView::handleTickEvent() {


		tickCounter++;

		if (tickCounter % 60 == 0) {
			if (++digitalSeconds >= 60) {
				digitalSeconds = 0;
				if (++digitalMinutes >= 60) {
					digitalMinutes = 0;
					if (++digitalHours >= 24) {
						digitalHours = 0;

						if (++digitalDays >= 365) {
							digitalDays = 0;
						}
					}

				}
			}

			digitalClock.setTime24Hour(digitalHours, digitalMinutes, digitalSeconds);
	}

	if (g_tickCounter % 60 == 0)
	{
		if (++g_digitalSeconds >= 60) {
						g_digitalSeconds = 0;
		}

		if(g_digitalSeconds == 5)
		{
			 g_digitalSeconds = 0;

		}
	}

	if (!boot_status_busy && g_boot_status.update_status == boot_status_update_start)
	{
		boot_status_busy = true;

		Status_editor(g_boot_status.last_status, bootloaderStatusBuffer, BOOTLOADERSTATUS_SIZE, bootloaderStatus);
		Update_progress(boxProgress1, g_boot_status.last_status_progress);

		g_boot_status.update_status = boot_status_update_done;
		boot_status_busy = false;
	}



}


void screenView::Status_editor( const char* Edit_string, touchgfx::Unicode::UnicodeChar *Buffer, uint16_t Buffer_size,  touchgfx::TextAreaWithOneWildcard Buffer_name)
{
	char Buffer_Hold[50];

	memset(&Buffer_Hold, 0, sizeof(Buffer_Hold));
	strcpy(Buffer_Hold, Edit_string);
	Unicode::strncpy(Buffer, ( const char*)Buffer_Hold, Buffer_size - 1);
	Buffer_name.invalidate();
}

void screenView::Update_progress(touchgfx::BoxProgress& progressBar, uint16_t val)
{
    progressBar.setValue(val);
    progressBar.invalidate();

}
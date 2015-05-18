#include <cyg/kernel/diag.h>
#include <cyg/hal/var_io.h>
#include <stdio.h>

#include "led.h"

cLED* cLED::__instance = NULL;

void cLED::init(cyg_uint32 * pinNumbers, cyg_uint8 pinCount)
{
	diag_printf("Initializing cLED\n");
	if(!__instance)
	{
		__instance = new cLED(pinNumbers, pinCount);
	}
}

cLED* cLED::get()
{
	return __instance;
}

cLED::cLED(cyg_uint32 * pinSpecs, cyg_uint8 pinCount) : mLEDCnt(pinCount)
{
	mLEDList = new cyg_uint32[mLEDCnt];
	memcpy(mLEDList, pinSpecs, sizeof(cyg_uint32) * mLEDCnt);

	setupPorts(mLEDList, mLEDCnt);

	animateTest();

	cyg_thread_create(LED_PRIORITY,
			cLED::led_thread,
			(cyg_addrword_t)this,
			(char *)"heartBeat",
			mLEDStack,
			LED_STACK_SIZE,
			&mLEDThreadHandle,
			&mLEDThread);
	cyg_thread_resume(mLEDThreadHandle);

}

void cLED::setupPorts(cyg_uint32 * pinSpecs, cyg_uint8 count)
{
	for (int k = 0; k < count; k++)
	{
		CYGHWR_HAL_STM32_GPIO_SET (pinSpecs[k]);
	}
}

void cLED::led_thread(cyg_addrword_t args)
{
	cLED* _this = (cLED*)args;

	for(;;)
	{
		_this->animateIdle();
	}
}

void cLED::animateIdle()
{
	for(cyg_uint8 k = 0; k < 4; k++)
	{
		setLED(k, 1);
		cyg_thread_delay(20);
		setLED(k, 0);
		cyg_thread_delay(20);
	}

	for(cyg_uint8 k = 2; k > 0; k--)
		{
			setLED(k, 1);
			cyg_thread_delay(20);
			setLED(k, 0);
			cyg_thread_delay(20);
		}
}

void cLED::animateTest()
{
	for (cyg_uint8 k = 0; k < mLEDCnt; k++)
		setLED(k,1);

	cyg_thread_delay(50);

	for (cyg_uint8 k = 0; k < mLEDCnt; k++)
		setLED(k,0);

	cyg_thread_delay(50);
}

void cLED::setLED(cyg_uint8 led, bool state)
{
	if(led >= mLEDCnt)
		return;

	if(mLEDList[led] == 0xFFFFFFFF)
		return;

	CYGHWR_HAL_STM32_GPIO_OUT (mLEDList[led], state);

}

#ifndef LED_H_
#define LED_H_
#include <cyg/kernel/kapi.h>
#include "definitions.h"

class cLED
{
	cLED(cyg_uint32 * pinSpecs, cyg_uint8 pinCount);
	static cLED* __instance;

	cyg_uint8 mLEDCnt;
	cyg_uint32 * mLEDList;

	cyg_uint8 mLEDStack[LED_STACK_SIZE];
	cyg_thread mLEDThread;
	cyg_handle_t mLEDThreadHandle;
	static void led_thread(cyg_addrword_t args);


	void setupPorts(cyg_uint32 * pinSPecs, cyg_uint8 count);

	void setLED(cyg_uint8, bool);
	void animateTest();
	void animateIdle();


public:
	static void init(cyg_uint32 * pinNumbers, cyg_uint8 pinCount);
	static cLED* get();


	void showIO(cyg_uint8, bool);
};

#endif /* LED_H_ */

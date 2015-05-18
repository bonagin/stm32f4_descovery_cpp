#ifndef ALARM_H_
#define ALARM_H_
#include "term.h"

#define PORTA_INPUT(__x)	__x
#define PORTB_INPUT(__x)	__x + 16
#define PORTC_INPUT(__x)	__x + 32
#define PORTD_INPUT(__x)	__x + 48
#define PORTE_INPUT(__x)	__x + 64
#define PORTF_INPUT(__x)	__x + 80

class cInput
{
	static cInput* __instance;
	bool mAlarmStart;
	cInput(cyg_uint32* portSpec, cyg_uint8 portCount);

	cyg_uint8 mInputCnt;
	cyg_uint32* mInputList;

	cyg_interrupt* mPDx_Interrupt;
	cyg_handle_t* mPDx_IntHandle;
	static cyg_uint32 handleISR(cyg_vector_t vector,cyg_addrword_t data);
	static void handleDSR(cyg_vector_t vector,cyg_uint32 count,cyg_addrword_t data);

	void setupPorts(cyg_uint32* ports, cyg_uint8 count);
	void setupInterrupts(cyg_uint32* ports, cyg_uint8 count);

public:
	static void init(cyg_uint32* portSpec, cyg_uint8 portCount);
	static cInput* get();

	void start();

	cyg_uint8 getPortCount(){ return mInputCnt; };
	bool getPortState(cyg_uint8);

	static void showInputs(cTerm & t,int argc,char *argv[]);
};

#endif /* ALARM_H_ */

#include <cyg/kernel/diag.h>
#include <cyg/kernel/kapi.h>
#include <stdio.h>
#include <time.h>

#include "input_port.h"
#include "definitions.h"
#include "utils.h"
#include "stm32cpu.h"
#include "logger.h"

cInput* cInput::__instance = NULL;

static const cyg_uint32 interrupt_exti[] =
{
	CYGNUM_HAL_INTERRUPT_EXTI0, //0
	CYGNUM_HAL_INTERRUPT_EXTI1,	//1
	CYGNUM_HAL_INTERRUPT_EXTI2, //2
	CYGNUM_HAL_INTERRUPT_EXTI3,	//3
	CYGNUM_HAL_INTERRUPT_EXTI4,	//4
	CYGNUM_HAL_INTERRUPT_EXTI5,	//5
	CYGNUM_HAL_INTERRUPT_EXTI6,	//6
	CYGNUM_HAL_INTERRUPT_EXTI7,	//7
	CYGNUM_HAL_INTERRUPT_EXTI8,	//8
	CYGNUM_HAL_INTERRUPT_EXTI9,	//9
	CYGNUM_HAL_INTERRUPT_EXTI10,//10
	CYGNUM_HAL_INTERRUPT_EXTI11,//11
	CYGNUM_HAL_INTERRUPT_EXTI12,//12
	CYGNUM_HAL_INTERRUPT_EXTI13,//13
	CYGNUM_HAL_INTERRUPT_EXTI14,//14
	CYGNUM_HAL_INTERRUPT_EXTI15,//15
};


/**
 * Initialize input ports
 *
 * @param portNumbers	A list of the input port specs
 * @param portCount		Number of ports in the list
 */
void cInput::init(cyg_uint32* portSpec, cyg_uint8 portCount)
{
	diag_printf("Initializing cInput\n");

	if(!__instance)
	{
		__instance = new cInput(portSpec, portCount);
	}
}

cInput* cInput::get()
{
	return __instance;
}

cInput::cInput(cyg_uint32* portSpec, cyg_uint8 portCount) : mInputCnt(portCount)
{
	mAlarmStart = false;

	mPDx_IntHandle = new cyg_handle_t[mInputCnt];
	mPDx_Interrupt = new cyg_interrupt[mInputCnt];
	mInputList = new cyg_uint32[mInputCnt];
	memcpy(mInputList, portSpec, sizeof(cyg_uint32) * mInputCnt);

	setupPorts(mInputList, mInputCnt);
	setupInterrupts(mInputList, mInputCnt);
}

void cInput::setupPorts(cyg_uint32* ports, cyg_uint8 count)
{
	for (int k = 0; k < count; k++)
	{
		CYGHWR_HAL_STM32_GPIO_SET (ports[k]);
	}
}

void cInput::setupInterrupts(cyg_uint32* ports, cyg_uint8 count)
{
	cyg_uint32 pin;

	//create and mask interrupts for all the buttons of the display
	for(int k = 0; k < count; k++)
	{
		pin = CYGHWR_HAL_STM32_GPIO_BIT(ports[k]);
		HAL_WRITE_UINT32( CYGHWR_HAL_STM32_EXTI + CYGHWR_HAL_STM32_EXTI_IMR, (1 << pin));

		cyg_uint32 exti = interrupt_exti[pin];

		cyg_interrupt_mask(exti);
		cyg_interrupt_create(exti,
				7,
				(cyg_addrword_t)k,
				cInput::handleISR,
				cInput::handleDSR,
				&mPDx_IntHandle[k],
				&mPDx_Interrupt[k]);

		cyg_interrupt_attach(mPDx_IntHandle[k]);
		cyg_interrupt_unmask(exti);
	}
}


void cInput::start()
{
	cyg_uint32 reg32;
	//enable trigger edge of interrupt
	HAL_READ_UINT32( CYGHWR_HAL_STM32_EXTI + CYGHWR_HAL_STM32_EXTI_RTSR, reg32 );


	for (int k = 0; k < mInputCnt; k++)
	{
		cyg_uint32 pin = CYGHWR_HAL_STM32_GPIO_BIT(mInputList[k]);
		reg32 |= 1 << pin;
	}


	HAL_WRITE_UINT32( CYGHWR_HAL_STM32_EXTI + CYGHWR_HAL_STM32_EXTI_RTSR, reg32 );
	HAL_WRITE_UINT32( CYGHWR_HAL_STM32_EXTI + CYGHWR_HAL_STM32_EXTI_FTSR, reg32);

	//wait a while for interrupts to stabilize
	cyg_thread_delay(500);
}

cyg_uint32 cInput::handleISR(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_interrupt_mask(vector);

   HAL_DELAY_US(20000);

    cyg_interrupt_acknowledge(vector);


   	return(CYG_ISR_CALL_DSR);

}

void cInput::handleDSR(cyg_vector_t vector,cyg_uint32 count,cyg_addrword_t data)
{
	cyg_uint8 input  = (cyg_uint8)data;

	if(__instance)
		diag_printf("\nDoor %d %s",input + 1,__instance->getPortState(input)?"OPEN":"CLOSED");

	time_t * t;

	struct tm* tStruct = localtime(t);

	time_t now = time(NULL);
	diag_printf(" ON %s\n",asctime(localtime(&now)));

	cyg_uint8 log[4];

	cyg_uint8 dir  =  0X00;
	cyg_uint8 day  = tStruct->tm_wday;
	cyg_uint8 hour = tStruct->tm_hour;
	cyg_uint8 min  = tStruct->tm_min;

	if(__instance->getPortState(input)){dir = 0x01;}else{dir = 0x00;}
	log[0] = min;
	log[1] = hour;
	log[2] = day;
	log[3] = dir;

	cyg_uint32 log32 = *((cyg_uint32*)log);

//	cyg_uint32 log32 = log[3]<<24 | log[2]<<16 | log[1]<<8 | log[0];

	cLOG::get()->write_log(log32);

    for(int x = 0; x < 4; x++)
    {
    	diag_printf("log[%d] = 0x%02X\n",x,log[x]);
    }

    diag_printf("log  = 0x%08X\n",log32);

	cyg_interrupt_unmask(vector);
}

bool cInput::getPortState(cyg_uint8 input)
{
	cyg_uint32 pin = CYGHWR_HAL_STM32_GPIO_BIT(mInputList[input]);
	cyg_uint32 port = CYGHWR_HAL_STM32_GPIO_PORT(mInputList[input]);
	cyg_uint32 reg32;
	HAL_READ_UINT32( port + CYGHWR_HAL_STM32_GPIO_IDR, reg32 );

	if(reg32 & (1 << pin))
		return true;

	return false;
}

void cInput::showInputs(cTerm & t,int argc,char *argv[])
{
	t << YELLOW("INPUT status:\n");
	for(cyg_uint8 k = 0; k < __instance->mInputCnt; k++)
	{
		if( __instance->getPortState(k))
			t << t.format(GREEN("\tP%d: 1\n"), k);
		else
			t << t.format(RED("\tP%d: 0\n"), k);
	}
}

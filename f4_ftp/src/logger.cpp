/*
 * logger.cpp
 *
 *  Created on: 20 May 2015
 *      Author: Bongani
 */
#include <cyg/kernel/diag.h>
#include <cyg/hal/var_io.h>
#include <stdio.h>
#include "stm32cpu.h"
#include "logger.h"


cLOG* cLOG::__instance = NULL;

cyg_uint32 start_address = 0x08040000;
cyg_uint32 last_address;

void cLOG::init()
{
	diag_printf("Initializing the LOGGER\n");
	if(!__instance)
	{
		__instance = new cLOG();
	}
}

cLOG* cLOG::get()
{
	return __instance;
}

cLOG::cLOG()
{

	cyg_thread_create(LOG_PRIORITY,
			cLOG::log_thread,
			(cyg_addrword_t)this,
			(char *)"LOGGER",
			mLOGStack,
			LOG_STACK_SIZE,
			&mLOGThreadHandle,
			&mLOGThread);
	cyg_thread_resume(mLOGThreadHandle);

}


void cLOG::log_thread(cyg_addrword_t args)
{
	cLOG* _this = (cLOG*)args;

	for (;;)
	{
		cyg_thread_delay(100);
	}
}

void cLOG::write_log(cyg_uint32 log)
{

read:

	if(stm32cpu::readflash(start_address) == 0xFFFFFFFF)
	{
		stm32cpu::witeflash(log,start_address);
	}
	else
	{
		start_address = start_address + 4;
		last_address = start_address;
		goto read;
	}

}

void cLOG::read_logs(cyg_uint32 start_address_, cyg_uint32 last_address_)
{

	if(start_address_ == 0x00){start_address_ = 0x08040000;}
	int i = 0;

read2:
    cyg_uint32 log = stm32cpu::readflash(start_address_);
    i ++;

	if(start_address_ == last_address_)
	{
		diag_printf("No more logs Available\n");
	}
	else
	{
		if(!(log == 0xFFFFFFFF))
		{
			if(((cyg_uint8)(log >> 24) & 0xff) == 0x00)
			{
				diag_printf("log #%d - Open on ",i);
			}
			else if(((cyg_uint8)(log >> 24) & 0xff) == 0x01)
			{
				diag_printf("log #%d - Closed on ",i);
			}

			if(((cyg_uint8)(log >> 16) & 0xff) == 0x00)
			{
				diag_printf("Sun at ");
			}
			else if(((cyg_uint8)(log >> 16) & 0xff) == 0x01)
			{
				diag_printf("Mon at ");
			}
			else if(((cyg_uint8)(log >> 16) & 0xff) == 0x02)
			{
				diag_printf("Tues at ");
			}
			else if(((cyg_uint8)(log >> 16) & 0xff) == 0x03)
			{
				diag_printf("Wed at ");
			}
			else if(((cyg_uint8)(log >> 16) & 0xff) == 0x04)
			{
				diag_printf("Thurs at ");
			}
			else if(((cyg_uint8)(log >> 16) & 0xff) == 0x05)
			{
				diag_printf("Fri at ");
			}
			else if(((cyg_uint8)(log >> 16) & 0xff) == 0x06)
			{
				diag_printf("Sat at ");
			}

			cyg_uint8 hour = (cyg_uint8)(log >> 8) & 0xff;
			cyg_uint8 min  = (cyg_uint8)(log >> 0) & 0xff;

			diag_printf("%dh%d\n",(int)hour,(int)min);

			start_address_ = start_address_ + 4;
			goto read2;
		}
		else
		{
			diag_printf("No more logs Available\n");
		}
	}

}

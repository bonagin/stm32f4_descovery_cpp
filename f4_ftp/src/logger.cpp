/*
 * logger.cpp
 *
 *  Created on: 20 May 2015
 *      Author: Bongani
 */
#include <cyg/kernel/diag.h>
#include <cyg/hal/var_io.h>
#include <stdio.h>
#include <time.h>

#include "stm32cpu.h"
#include "logger.h"
#include "usb_term.h"
#include "term.h"



char usb_data[100];

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

void cLOG::write_log(cyg_uint8 dir_)
{

	time_t * t;

	struct tm* tStruct = localtime(t);

	time_t now = time(NULL);
	diag_printf(" ON %s\n",asctime(localtime(&now)));

	cyg_uint8 log[4];

	cyg_uint8 day  = tStruct->tm_wday;
	cyg_uint8 hour = tStruct->tm_hour;
	cyg_uint8 min  = tStruct->tm_min;

	log[0] = min;
	log[1] = hour;
	log[2] = day;
	log[3] = dir_;

	cyg_uint32 log32 = *((cyg_uint32*)log);

//	cyg_uint32 log32 = log[3]<<24 | log[2]<<16 | log[1]<<8 | log[0];

    for(int x = 0; x < 4; x++)
    {
    	diag_printf("log[%d] = 0x%02X\n",x,log[x]);
    }

    diag_printf("log  = 0x%08X\n",log32);

    sprintf(usb_data, "DIR:%02X\nDAY:%02X\nHOR:%02X\nMIN:%02X\n", dir_, day, hour, min);

    usbTerm::get()->write(usb_data, 100);

read:

	if(stm32cpu::readflash(start_address) == 0xFFFFFFFF)
	{
		stm32cpu::witeflash(log32,start_address);
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
    cyg_uint32 log_ = stm32cpu::readflash(start_address_);
    i ++;

	if(start_address_ == last_address_)
	{
		diag_printf("No more logs Available\n");
	}
	else
	{
		if(!(log_ == 0xFFFFFFFF))
		{
			cyg_uint8 dir = ((cyg_uint8)(log_ >> 24));
			if((dir & 0xff) == 0x00)
			{
				diag_printf("log_ #%d - Open on ",i);
			}
			else if((dir & 0xff) == 0x01)
			{
				diag_printf("log_ #%d - Closed on ",i);
			}

			cyg_uint8 day = ((cyg_uint8)(log_ >> 16) & 0xff);
			if((day & 0xff) == 0x00)
			{
				diag_printf("Sunday at ");
			}
			else if((day & 0xff) == 0x01)
			{
				diag_printf("Monday at ");
			}
			else if((day & 0xff) == 0x02)
			{
				diag_printf("Tuesday at ");
			}
			else if((day & 0xff) == 0x03)
			{
				diag_printf("Wednesday at ");
			}
			else if((day & 0xff) == 0x04)
			{
				diag_printf("Thursday at ");
			}
			else if((day & 0xff) == 0x05)
			{
				diag_printf("Friday at ");
			}
			else if((day & 0xff) == 0x06)
			{
				diag_printf("Saturday at ");
			}

			cyg_uint8 hour = (cyg_uint8)(log_ >> 8) & 0xff;
			cyg_uint8 min  = (cyg_uint8)(log_ >> 0) & 0xff;

			diag_printf("%dh%d\n",(int)hour,(int)min);

		    sprintf(usb_data,"LOG:%d\nDIR:%02X\nDAY:%02X\nHOR:%02X\nMIN:%02X\n",i,dir, day, hour, min);

		    usbTerm::get()->write(usb_data, 100);

			start_address_ = start_address_ + 4;
			goto read2;
		}
		else
		{
			diag_printf("No more logs Available\n");
		}
	}

}

#include <cyg/kernel/diag.h>
#include <cyg/io/ttyio.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "TermCMD.h"
#include "utils.h"
#include "uart_term.h"

uartTerm * uartTerm::__instance = NULL;

void uartTerm::init(const char * dev, cyg_uint32 b_size, const char * const prompt_str)
{
    if (__instance == NULL)
    {
        __instance = new uartTerm(dev,b_size,prompt_str);
    }
}

uartTerm::uartTerm(const char * dev,cyg_uint32 b_size,const char * const prompt_str) : cTerm(b_size, prompt_str)
{
    Cyg_ErrNo err;

    cyg_uint32 len = strlen(dev)+1;
    mDev = new char[len];
    strcpy(mDev,dev);

    mBuffSize = b_size;
    mPrevBuff = new char[mBuffSize];
    mPrevBuff[0] = 0;
    mCMDidx = 0;



    err = cyg_io_lookup(mDev,&mDevHandle);

    diag_printf("TERM: Lookup error %s \n",strerror(-err));

    cyg_tty_info_t tty_info;
    len = sizeof(tty_info);
    cyg_io_get_config(mDevHandle,
                      CYG_IO_GET_CONFIG_TTY_INFO,
                      &tty_info,
                      &len);

    //diag_printf("TERM: TTY in_flags = 0x%08X, out_flags = 0x%08X\n",tty_info.tty_in_flags,tty_info.tty_out_flags);

    tty_info.tty_in_flags = (CYG_TTY_IN_FLAGS_CR | CYG_TTY_IN_FLAGS_ECHO);

    cyg_io_set_config(mDevHandle,
                      CYG_IO_SET_CONFIG_TTY_INFO,
                      &tty_info,
                      &len);

    banner();
    prompt();

    cyg_thread_create(TERM_PRIORITY,
                      uartTerm::term_thread_func,
                      (cyg_addrword_t)this,
                      (char *)"uartTerm",
                      mStack,
                      TERM_STACK_SIZE,
                      &mThreadHandle,
                      &mThread);

    cyg_thread_resume(mThreadHandle);
}

void uartTerm::write(const char * string, cyg_uint32 len)
{
	cyg_io_write(mDevHandle, string, &len);
}


void uartTerm::term_thread_func(cyg_addrword_t arg)
{
    uartTerm * t = (uartTerm *)arg;


    for (;;)
    {
        t->run();
    }
}

void uartTerm::run(void)
{
    mCMDidx = mBuffSize;
    cyg_io_read(mDevHandle,mCMDbuff,&mCMDidx);

    mCMDbuff[mCMDidx-1] = 0;

    if ( mCMDidx >= 1 )
    {
    	process_command_line();
    }
    prompt();
}



void uartTerm::banner()
{
    *this<<"Terminal started on device \""<<mDev<<"\"\n";
}

cyg_bool uartTerm::wasArrow()
{
	if(mCMDidx > 3)
	{
		if(mCMDbuff[0] == 0x1B && mCMDbuff[1] == 0x5B)
			return true;
	}
	return false;
}

void uartTerm::process_command_line(void)
{

    char *argv[20];
    int argc = 20;

    //check for arrow presses
    if(wasArrow() && mPrevBuff[0])
    {
    	util_parse_params(mPrevBuff,argv,argc,' ',' ');

    	if ( argc )
    	{
    		TermCMD::process(*this,argc,argv);
    	}
    	return;
    }

    util_parse_params(mCMDbuff,argv,argc,' ',' ');

    if ( argc )
    {
    	memcpy(mPrevBuff, mCMDbuff, mCMDidx);
    	TermCMD::process(*this,argc,argv);
    }
}

uartTerm::~uartTerm()
{
    delete [] mDev;
}

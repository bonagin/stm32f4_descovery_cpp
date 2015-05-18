#ifndef _UART_TERM_H_
#define _UART_TERM_H_
#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/io/io.h>

#include "ksesTerminal_def.h"
#include "term.h"

class uartTerm : public cTerm
{
private:
    static uartTerm * __instance;
    cyg_uint32 mBuffSize;
    cyg_io_handle_t mDevHandle;
    char * mDev;
    char * mPrevBuff;

    cyg_uint8 mStack[TERM_STACK_SIZE];
    cyg_thread mThread;
    cyg_handle_t mThreadHandle;
    static void term_thread_func(cyg_addrword_t arg);

    uartTerm(const char * dev, cyg_uint32 b_size, const char * const prompt_str);

    cyg_bool wasArrow();
    void process_command_line();
    void run(void);

    void write(const char * string, cyg_uint32 len);
    void banner();

public:
    static void init(const char * dev, cyg_uint32 b_size, const char * const prompt_str);

    virtual ~uartTerm();
};

#endif  //Include Guard

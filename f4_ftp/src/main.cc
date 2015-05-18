#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_diag.h>
#include <cyg/kernel/diag.h>

#include "init.h"

void * _impure_ptr;	// g++ compatibility

extern "C" void cyg_user_start(void);


void cyg_user_start(void)
{
	diag_printf("Hello %s\n", __DATE__);

	cInit::init();
}

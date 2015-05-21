#ifndef DEFINITIONS_H
#define	DEFINITIONS_H
#include <cyg/hal/hal_arch.h>

//Stack and thread definitions
#define INIT_PRIORITY		19
#define LED_PRIORITY		18
#define LOG_PRIORITY		22

//Stack thread sizes
#define INIT_STACK_SIZE		CYGNUM_HAL_STACK_SIZE_TYPICAL
#define LED_STACK_SIZE		CYGNUM_HAL_STACK_SIZE_MINIMUM
#define LOG_STACK_SIZE      CYGNUM_HAL_STACK_SIZE_MINIMUM


#endif	/* DEFINITIONS_H */


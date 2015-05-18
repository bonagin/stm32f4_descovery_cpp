#ifndef KSES_TERMINAL_DEF_H
#define	KSES_TERMINAL_DEF_H
#include <cyg/hal/hal_arch.h>

//Stack and thread definitions
#define USB_PRIORITY		25
#define TERM_PRIORITY		25

//Stack thread sizes
#define USB_STACK_SIZE		CYGNUM_HAL_STACK_SIZE_TYPICAL
#define TERM_STACK_SIZE		CYGNUM_HAL_STACK_SIZE_TYPICAL

#endif	/* DEFINITIONS_H */


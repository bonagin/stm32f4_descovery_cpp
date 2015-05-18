#include <cyg/kernel/diag.h>
#include <TermCMD.h>

#include "stm32cpu.h"

TermCMD::cmd_table_t TermCMD::mCmdTable [] =
{
	{"SYSTEM"	,0,0,0},
    {"h"	, "",			"Show this help info", TermCMD::help},
    {"help"	, "",			"Show this help info", TermCMD::help},
    {"dt"	, "",			"Dump Thread Info", System::handle},
    {"time"	, "",			"Current system time", System::handle},
    {"settime"	, "<yyyy mm dd HH MM SS>",			"Set system time", System::setTime},
    {"reset"	, "",			"Reset processor", System::reset},
	{"FLASH"	,0,0,0},
	{"flash", "<read/write/erase/eraseall> <address> <value>",			"Read/write/erase the flash" , stm32cpu::flashCmd},
	{"STM32"	,0,0,0},
	{"pstatus"	, "",			"Reset processor", stm32cpu::pStatus},

    {0, 0, 0, 0}
};


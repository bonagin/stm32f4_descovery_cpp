#ifndef STM32CPU_H_
#define STM32CPU_H_

#include "term.h"

typedef enum
{
	  FLASH_BUSY = 1,
	  FLASH_ERROR_PGS,
	  FLASH_ERROR_PGP,
	  FLASH_ERROR_PGA,
	  FLASH_ERROR_WRP,
	  FLASH_ERROR_PROGRAM,
	  FLASH_ERROR_OPERATION,
	  FLASH_COMPLETE

}flash_status;

class stm32cpu
{

public:

	static cyg_uint64 uniqueID();
	static cyg_uint16 flashSize();
	static cyg_uint32 readflash(cyg_uint32 Address);
	static void witeflash(cyg_uint32 Data, cyg_uint32 Address);
	static void eraseflash(cyg_uint32 Sector);
	static void eraseallsectors();
	static void flashunlock();
	static void lock();
	static flash_status getflashstatus();
	static cyg_uint16 ramSize();
	static void info();
	static void sysResetStatus();
	static void pStatus(cTerm & term, int argc,char * argv[]);
	static void flashCmd(cTerm & t,int argc,char *argv[]);
};

#endif /* STM32CPU_H_ */

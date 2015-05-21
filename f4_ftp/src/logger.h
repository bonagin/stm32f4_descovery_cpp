
#ifndef LOGGER_H_
#define LOGGER_H_

#include <cyg/kernel/kapi.h>
#include "definitions.h"
#include "term.h"

class cLOG
{
	static cLOG* __instance;

	cyg_uint8 mLOGStack[LOG_STACK_SIZE];
	cyg_thread mLOGThread;
	cyg_handle_t mLOGThreadHandle;
	static void log_thread(cyg_addrword_t args);
	static void read_logs();

	cLOG();

public:
	static void init();
	static cLOG* get();
	static void write_log(cyg_uint32 log);
};



#endif /* LOGGER_H_ */

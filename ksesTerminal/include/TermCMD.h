#ifndef TERMCMD_H_
#define TERMCMD_H_
#include <cyg/hal/hal_tables.h>
#include <cyg/kernel/kapi.h>
#include "term.h"

class TermCMD
{
	typedef void (*func_t)(cTerm & t,int argc,char *argv[]);

public:
	    typedef struct
	    {
	    	char* cmd;
	    	char* argDesc;
	    	char* desc;
	        func_t f;
	    } cmd_table_t;
	    static cmd_table_t mCmdTable[];

	TermCMD();

	static void process(cTerm & term, int argc,char * argv[]);
	static void help(cTerm & t,int argc,char *argv[]);
};

class System
{
public:
	static void handle(cTerm & t,int argc,char *argv[]);
	static void nvmBuff(cTerm & t,int argc,char *argv[]);
	static void config(cTerm & t,int argc,char *argv[]);
	static void reset(cTerm & t,int argc,char *argv[]);
	static void setTime(cTerm & term, int argc,char * argv[]);
	static void setManual(cTerm & term, int argc,char * argv[]);
	static void setDynamic(cTerm & term, int argc,char * argv[]);

	static void setDebugLvl(cTerm & term, int argc,char * argv[]);
};


#endif /* TERMCMD_H_ */

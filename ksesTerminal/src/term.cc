#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "term.h"

cTerm::cTerm(cyg_uint32 b_size, const char * const prompt_str)
{
	 mPrompt = prompt_str;
	 mCMDbuff = new char[b_size];
	 mCMDidx = 0;
}

void cTerm::prompt()
{
	*this<<"\n"<<mPrompt;
}

cTerm& cTerm::operator <<(const char *str)
{
    cyg_uint32 len = strlen(str);

    write(str,len);

    return(*this);
}
cTerm& cTerm::operator <<(int i)
{
    char str[20];
    sprintf(str,"%d",i);
    cyg_uint32 len = strlen(str);

    write(str,len);

    return(*this);
}

cTerm& cTerm::operator <<(unsigned char c)
{
    char str[6];
    sprintf(str,"0x%02X",c);
    cyg_uint32 len = strlen(str);

    write(str,len);

    return(*this);
}

cTerm& cTerm::operator <<(unsigned int i)
{
    char str[12];
    sprintf(str,"0x%08X",i);
    cyg_uint32 len = strlen(str);

    write(str,len);

    return(*this);
}
cTerm& cTerm::operator <<(void * p)
{
    *this<<(unsigned int)p;
    return(*this);
}


char * cTerm::format(const char *f,...)
{
    va_list vl;
    va_start(vl,f);
    vsprintf(mTempBuf,f,vl);
    va_end(vl);

    return mTempBuf;
}

cTerm::~cTerm()
{
	delete mCMDbuff;
}


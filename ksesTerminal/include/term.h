#ifndef _TERM_H_
#define _TERM_H_

class cTerm
{
	char mTempBuf[128];

protected:
    const char * mPrompt;
    char * mCMDbuff;
    cyg_uint32 mCMDidx;

    void prompt();

public:
    cTerm(cyg_uint32 b_size, const char * const prompt_str);
    virtual ~cTerm();

    cTerm& operator<<(void *);
    cTerm& operator<<(const char *);
    cTerm& operator<<(int);
    cTerm& operator<<(unsigned int);
    cTerm& operator<<(unsigned char);

    char * format(const char *f,...);

    virtual void banner() = 0;
    virtual void write(const char * string, cyg_uint32 len) = 0;

};

#endif  //Include Guard

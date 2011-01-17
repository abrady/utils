/***************************************************************************
 *     Copyright (c) 2008-2008, Aaron Brady
 *     All Rights Reserved
 *     Confidential Property of Aaron Brady
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#include "abstd.h"
#include "log.h"
#include "abassert.h"

#include "windows.h"

// called when an assert is already happening
BOOL abassert(char const *expr, char const *msg DBG_PARMS)
{
    loglevel_push(0);
    printf("assert %s(%d):%s, %s\n",caller_fname,line,expr,msg);
    _DbgBreak();   // @todo -AB: wire up a real handler :10/22/08
    loglevel_pop();
    return FALSE;
}


void abErrorv(BOOL fatal, char *file, int line, const char *format, va_list ap)
{
	// todo
    printf("%s(%i): ",file,line);
	vprintf(format,ap);
    if(fatal)
        abassert("",format,file,line);
}


void abErrorDbg(char *file, int line, FORMAT format, ...)
{
    va_list args;
    va_start(args,format);
    abErrorv(FALSE,file,line,format,args);
    va_end(args);
}

void abErrorFatalDbg(char *file, int line, FORMAT format, ...)
{
    va_list args;
    va_start(args,format);
    abErrorv(TRUE,file,line,format,args);
    va_end(args);
}

int last_error()
{
#if WIN32
    return GetLastError();
#endif
}


char *last_error_str()
{ 
	static char *buf = NULL;
	if(buf)
		LocalFree(buf);
	
	if(!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
					  | FORMAT_MESSAGE_FROM_SYSTEM,
					  NULL, // lpSource
                      GetLastError(),
					  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					  (LPTSTR)&buf,0,
					  NULL))
		return NULL;
	return buf;
}

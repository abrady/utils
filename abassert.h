/***************************************************************************
 *     Copyright (c) 2008-2008, Aaron Brady
 *     All Rights Reserved
 *     Confidential Property of Aaron Brady
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef abassert_H
#define abassert_H

#include "abstd.h"
ABHEADER_BEGIN

#define assert_static(e)		extern char compiletime_assert[(e)?1:-1]
#define assert_infunc_static(e)	block_as_line(assert_static(e);)
#define assert_static_inexpr(e)	(1 / ((e)?1:0))									// e.g. asi(foo),expr

BOOL abassert(char const *expr, char const *msg DBG_PARMS);
void abErrorDbg(char *file, int line, FORMAT format, ...);
NORETURN abErrorFatalDbg(char *file, int line, FORMAT format, ...);
#define abError(format,...) abErrorDbg(__FILE__,__LINE__,format,__VA_ARGS__)
#define abErrorFatal(format,...) abErrorFatalDbg(__FILE__,__LINE__,format,__VA_ARGS__)

#define assertm(cond,msg,...)	(void)(!!(cond) || (abErrorFatalDbg(__FILE__,__LINE__,msg,__VA_ARGS__),0))

#undef assert
// #define assert(X)			(void)(!!(X)) || (IsDebuggerPresent()?abassert(#X DBG_PARMS_CALL) :(_wassert(_CRT_WIDE(#X), _CRT_WIDE(__FILE__), __LINE__), 0))
#define assert(X)				(void)(!!(X) || (abassert(#X, "assert failure" DBG_PARMS_INIT)))
#define assert_dbg(X)			(void)(!!(X) || (abassert(#X, "assert failure" DBG_PARMS_CALL)))

char *last_error_str();
int last_error();

#ifdef _DbgBreak
#	undef _DbgBreak
#endif
#define _DbgBreak() __debugbreak()

ABHEADER_END
#endif //abassert_H

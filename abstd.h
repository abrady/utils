/***************************************************************************
 *     Copyright (c) 2008-2008, Aaron Brady
 *     All Rights Reserved
 *     Confidential Property of Aaron Brady
 *
 * Module Description:
 * !!! Include this before all other include files !!!
 *
 ***************************************************************************/
#ifndef abstd_H
#define abstd_H

// way VC prints text dynamically
// #define _CRT_DEPRECATE_TEXT(_Text) __declspec(deprecated(_Text))

#ifdef __cplusplus
#	define C_DECL extern "C"
#	define C_DECL_BEGIN extern "C" {
#	define C_DECL_END }
#else
#	define C_DECL
#	define C_DECL_BEGIN
#	define C_DECL_END
#endif

#define ABHEADER_BEGIN C_DECL_BEGIN
#define ABHEADER_END C_DECL_END
#define ABDEF C_DECL
#define ABFUNC C_DECL

// standard includes
//#include <varargs.h>
#ifdef WIN32
#   if defined(_WINDOWS_) && defined (UTILS_LIB)
#      error "abstd not first included in file"
#   endif
#   define WIN32_LEAN_AND_MEAN
#   ifndef _WIN32_WINNT   // this enables newer win32 functionality e.g. IsDebuggerPresent
#      define _WIN32_WINNT 0x0400 // XP
#   endif
#   include <winsock2.h>
#   include <CodeAnalysis/SourceAnnotations.h> // for source annotations
#   include <windows.h>
#endif
#define _CRT_RAND_S
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
ABHEADER_BEGIN


// *************************************************************************
// warnings to ignore
// *************************************************************************

#if defined(WIN32)
#	pragma warning(disable:4127) // conditional expression is constant
#	pragma warning(disable:4267) // conversion from size_t to int
#	pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#	pragma warning(disable:6011) // malloc returns NULL check
//#	pragma warning(disable:6328) // 'char' passed as parameter '1' when 'unsigned char' is required in call to 'isxdigit' ab: needed, signed chars go negative, f-up lookup
#	pragma warning(disable:6255) // "_alloca indicates failure by raising a stack overflow exception. Consider using _malloca instead"
#endif

// ----------------------------------------
// useful defines

#define block_as_line(...)		do { __VA_ARGS__ } while(0)

#define ABINLINE __forceinline

#ifndef OPTIONAL
#define OPTIONAL
#endif

// ---------------------------------------- 
// allocator related

//#define OVERRIDE_MALLOC

#define DBG_PARMS_MBR const char *caller_fname; \
    int line
#define DBG_PARMS_MBR_ASSIGN(P) (P)->caller_fname = caller_fname;   \
    (P)->line = line
#define DBG_PARMS , const char *caller_fname, int line
#define DBG_PARMS_SOLO const char *caller_fname, int line
#define DBG_PARMS_CALL , caller_fname, line
#define DBG_PARMS_INIT , __FILE__, __LINE__
#define DBG_PARMS_INIT_SOLO __FILE__, __LINE__
#define DBG_PARMS_UNUSED caller_fname; line;

#ifndef NO_MALLOC_OVERRIDE

#if defined(WIN32)
#   include <stdlib.h>
#   undef _malloc_dbg
#   undef _calloc_dbg
#   undef _realloc_dbg
#   undef _free_dbg
#   include <crtdbg.h>
#   include <malloc.h>
#endif

// apparently these are defined so function pointers aren't used

#undef malloc
#undef calloc
#undef realloc
#undef free
#define malloc(size)		abmalloc_dbg(size DBG_PARMS_INIT)
#define calloc(num,size)	abcalloc_dbg(num,size DBG_PARMS_INIT)
#define realloc(data,size)	abrealloc_dbg(data,size DBG_PARMS_INIT)
#define free(data) abfree_dbg(data DBG_PARMS_INIT)
#undef strdup
#undef _strdup
#define strdup(a)  abstrdup(a DBG_PARMS_INIT) // malloc isn't called in strdup, so
#define _strdup(a) abstrdup(a DBG_PARMS_INIT) // hack in a custom one :(
char *abstrdup(const char *str DBG_PARMS);    // bonus: nulls pass through

void *abmalloc_dbg(size_t size DBG_PARMS);
void *abcalloc_dbg(size_t n, size_t s DBG_PARMS);
void *abrealloc_dbg(void *p, size_t s DBG_PARMS);
void abfree_dbg(void *p DBG_PARMS);

typedef struct TrackedAlloc
{
    char *file;
    int line;
    void *p;
} TrackedAlloc;

#define ABMALLOC_DBGCALL(N) abmalloc_dbg((N) DBG_PARMS_CALL)        
#define ABREALLOC_DBGCALL(P,N) abrealloc_dbg((P), (N) DBG_PARMS_CALL)

#else
#define ABMALLOC_DBGCALL(N) malloc(N)
#define ABREALLOC_DBGCALL(P,N) realloc((P), (N))
#endif // OVERRIDE_MALLOC

#define HFREE(hp) (((hp) && (*hp))?(free(*hp),(*hp)=0):NULL)
#define MALLOCT(T) (assert_static_inexpr(sizeof(T)!=sizeof(void*)),calloc(1,sizeof(T)))
#define MALLOCP(P) (assert_static_inexpr(sizeof(*(P))!=sizeof(void*)),calloc(1,sizeof(*(P))))
#ifdef UTILS_LIB
#   define MALLOCP_DBG_CALL(P) (assert_static_inexpr(sizeof(*(P))>4),abcalloc_dbg(sizeof(*(P)),1, caller_fname, line))
#else
#   define MALLOCP_DBG_CALL(P) (assert_static_inexpr(sizeof(*(P))>4),_calloc_dbg(sizeof(*(P)),1, _NORMAL_BLOCK, caller_fname, line))
#endif
#define DESTROY(p,T) (T ## _Destroy(p),(p) = 0)
//#define alloca _alloca
#ifdef _ALLOCA_S_THRESHOLD
#   define MALLOC_STACK_MAX _ALLOCA_S_THRESHOLD
#else
#   define MALLOC_STACK_MAX 1024
#endif 
#define malloc_stack(size) (((size) >= MALLOC_STACK_MAX) ? NULL : _alloca(size))



// ----------------------------------------
// our standard includes (after malloc handlers

#ifndef __BYTE_ORDER
#       define __LITTLE_ENDIAN 1
#       define __BYTE_ORDER __LITTLE_ENDIAN
#endif 

typedef unsigned char U8;
typedef signed char S8;
typedef volatile signed char VS8;
typedef volatile unsigned char VU8;

typedef unsigned short U16;
typedef short S16;
typedef volatile unsigned short VU16;
typedef volatile short VS16;

typedef int S32;
typedef unsigned int U32;
typedef volatile int VS32;
typedef volatile unsigned int VU32;

typedef unsigned __int64 U64;
typedef __int64 S64;
typedef volatile __int64 VS64;
typedef volatile unsigned __int64 VU64;

typedef float F32;
typedef volatile float VF32;

typedef U16 F16;
typedef VU16 VF16;

typedef volatile double VF64;
typedef double F64;

#define S32_MIN    (-2147483647i32 - 1) /* minimum signed 32 bit value */
#define S32_MAX      2147483647i32 /* maximum signed 32 bit value */
#define U32_MAX     0xffffffffui32 /* maximum unsigned 32 bit value */
#define U16_MAX		0xffffui16 /* maximum unsigned 32 bit value */
#define U8_MAX		0xffui8 /* maximum unsigned 32 bit value */

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

// are macros better? could do offset >> sizeof(*mem)*3
ABINLINE void bit_set(S32 *mem,S32 offset) { mem[offset >> 5] |= (1 << (offset & 31)); }
ABINLINE void bit_clr(S32 *mem,S32 offset) { mem[offset >> 5] &= ~(1 << (offset & 31)); }
ABINLINE BOOL bit_tst(S32 *mem,S32 offset) { return (mem[offset >> 5] & (1 << (offset & 31)));} 

#define IS_ARRAY(a)				((char*)(a) == (char*)&(a))
// #define ARRAY_SIZE_UNSAFE(n)	(sizeof(n) / sizeof((n)[0])) // I can't think of any (safe) reason for this
#ifdef ARRAY_SIZE
#	undef ARRAY_SIZE
#endif
#define ARRAY_SIZE(n)			(int)(sizeof(n) / ((sizeof(n)==sizeof(void*) || sizeof(n)==0) ? 0: sizeof((n)[0])))
#define ASTR(str)				str, ARRAY_SIZE(str)

#define ClearStruct(p)			memset((p), 0, sizeof(*(p)))
#define ClearStructs(p,count)	memset((p), 0, sizeof(*(p)) * (count))
#define ClearArray(x)			assert(IS_ARRAY(x)),memset((x), 0, sizeof(x))

#define SAFE_MEMBER(p, c1)					((p)?(p)->c1:0)
#define SAFE_MEMBER2(p, c1, c2)				(((p)&&(p)->c1)?(p)->c1->c2:0)
#define SAFE_MEMBER3(p, c1, c2, c3)			(((p)&&(p)->c1&&(p)->c1->c2)?(p)->c1->c2->c3:0)
#define SAFE_MEMBER4(p, c1, c2, c3, c4)		(((p)&&(p)->c1&&(p)->c1->c2&&(p)->c1->c2->c3)?(p)->c1->c2->c3->c4:0)

#define CopyStructs(dest,src,count)			memmove((dest),(src),sizeof((dest)[0]) * (count))
#define CopyStruct(dest,src)				CopyStructs(dest,src,1)

#define xcase		break;case
#define xdefault	break;default

// ----------------------------------------
// App verifier flags

/*********** For use with STRINGS  ***************/
#define NN_STR_MAKE					__checkReturn __notnull __post __nullterminated __post __valid
#define OPT_STR_MAKE				__checkReturn __maybenull __post __nullterminated __post __valid

#define NN_STR_GOOD					__notnull   __pre __valid __pre __nullterminated __post __nullterminated __post __valid
#define OPT_STR_GOOD				__maybenull __pre __valid __pre __nullterminated __post __nullterminated __post __valid

#define NN_STR_FREE					__notnull   __post __notvalid
#define OPT_STR_FREE				__maybenull __post __notvalid

#define FORMAT						[SA_FormatString(Style="printf")] const char *

#define NORETURN					__declspec(noreturn) void

#define VA_START(va, format)	{va_list va, *__vaTemp__ = &va;va_start(va, format)
#define VA_END()				va_end(*__vaTemp__);}

// *************************************************************************
// numeric 
// *************************************************************************

#ifndef MIN
#define MIN(a,b)	(((a)<(b)) ? (a) : (b))
#define MAX(a,b)	(((a)>(b)) ? (a) : (b))
#define MINMAX(a,min,max) ((a) < (min) ? (min) : (a) > (max) ? (max) : (a))
#endif

#if __BIG_ENDIAN
#define MAKE_TYPE_CODE(A,B,C,D) ((A<<24)|(B<<16)|(C<<8)|D)
#else
#define MAKE_TYPE_CODE(A,B,C,D) ((D<<24)|(C<<16)|(B<<8)|A)
#endif
ABINLINE U32 make_type_code_from_str(char *S) {U32 A = S[0]; U32 B = S[0]?S[1]:0; U32 C = (S[0]&&S[1])?S[2]:0; U32 D = (S[0]&&S[1]&&S[2])?S[3]:0; U32 ret = MAKE_TYPE_CODE(A,B,C,D); return ret;}

// ------------------------------------------------------------
// ranges

#define INRANGE( var, min, max ) ((var) >= min && (var) < max)
#define INRANGE0(var, max) INRANGE(var, 0, max)
#define AINRANGE( var, array ) INRANGE0(var, ARRAY_SIZE(array) )
#define APINRANGE( index, eavar ) (INRANGE0( index, ap_size( &(eavar))))
#define ACHRINRANGE( index, eaintvar ) (INRANGE0( index, achr_size(&(eaintvar))))

// *************************************************************************
// string
// *************************************************************************

#define streq(a,b) (_stricmp(a, b) == 0)

ABINLINE char* strstrAdvance(char *haystack, const char *needle)
{
	haystack = strstr(haystack, needle);
	return haystack ? haystack + strlen(needle) : NULL;
}

ABINLINE char* strBegins(char *haystack, const char *needle)
{
	int len = strlen(needle);
	if(_strnicmp(haystack, needle, len) == 0)
		return haystack + len;
	return NULL;
}

ABINLINE char* strEnds(char *haystack, const char *needle)
{
	int hlen = strlen(haystack);
	int nlen = strlen(needle);
	if(hlen >= nlen)
	{
		haystack += hlen - nlen;
		if(streq(haystack, needle))
			return haystack;
	}
	return NULL;
}

#define ch_isdigit(ch)	isdigit((unsigned char)(ch))
#define ch_isxdigit(ch)	isxdigit((unsigned char)(ch))
#define ch_isalnum(ch)	isalnum((unsigned char)(ch))
#define ch_isalpha(ch)	isalpha((unsigned char)(ch))

char *strstr_advance_len(char **s,char const *str, int len);
#define STRSTR_ADVANCE(S,STR) ((STR "this is a string"),strstr_advance_len(&S,STR,sizeof(STR)-1))

#define COPY_CLEAR(dst,src)	((dst) = (src), (src) = NULL, (dst))

#ifndef _STDTYPES_H
#	define strcpy(dst, src)			strcpy_safe(dst, ARRAY_SIZE(dst), src)
#	define strcat(dst, src)			strcat_safe(dst, ARRAY_SIZE(dst), src)
#	define sprintf(dst, fmt, ...)	sprintf_safe(dst, ARRAY_SIZE(dst), fmt, __VA_ARGS__)
#	define itoa(value, dst, radix)	itoa_safe(value, dst, ARRAY_SIZE(dst), radix)
#endif

#if WIN32
#	define strcpy_safe(dst, dst_size, src)			strcpy_s(dst, dst_size, src)
#	define strcat_safe(dst, dst_size, src)			strcat_s(dst, dst_size, src)
#	define sprintf_safe(dst, dst_size, fmt, ...)	sprintf_s(dst, dst_size, fmt, __VA_ARGS__)
#	define itoa_safe(value, dst, dst_size, radix)	_itoa_s(value, dst, dst_size, radix)
#else
#	error Safe string functions are undefined on this platform.
#endif

// *************************************************************************
//  
// *************************************************************************

#include "log.h"
#include "abassert.h"

typedef unsigned (*ThreadMainFp)(void *args);
#define thread_begin(thread_main,thread_args) _beginthreadex(0,0,thread_main,thread_args,0,0) // make sure to call CloseHandle! 

// *************************************************************************
//  Meta
// *************************************************************************

#if !defined(META) && !defined(NO_STD_META)
#define META(...)
#endif

ABHEADER_END
#endif //abstd_H

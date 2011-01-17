/***************************************************************************
 *     Copyright (c) 2008-2008, Aaron Brady
 *     All Rights Reserved
 *     Confidential Property of Aaron Brady
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef STR_H
#define STR_H

#include "abstd.h"
#include "Array.h"	// achr_X
ABHEADER_BEGIN

typedef char* Str;

#define Str_temp()									Str_temp_dbg(malloc_stack(1024), 1024 DBG_PARMS_INIT)
#define Str_clear(hstr)								Str_clear_dbg(hstr DBG_PARMS_INIT)
#define Str_len(hstr)								Str_len_dbg(hstr DBG_PARMS_INIT)
#define Str_printf(hstr, format, ...)				Str_printf_dbg(hstr DBG_PARMS_INIT, format, __VA_ARGS__)
#define Str_vprintf(hstr, format, args)				Str_vprintf_dbg(hstr DBG_PARMS_INIT, format, args)
#define Str_catf(hstr, format, ...)					Str_catf_dbg(hstr DBG_PARMS_INIT, format, __VA_ARGS__)
#define Str_vcatf(hstr, format, args)				Str_vcatf_dbg(hstr DBG_PARMS_INIT, format, args)

#define Str_catprintable(hstr, src)					Str_catprintable_dbg(hstr, src, ' ' DBG_PARMS_INIT)
#define Str_catprintableex(hstr, src, def)			Str_catprintable_dbg(hstr, src, def DBG_PARMS_INIT)
#define Str_caturi(hstr, src)						Str_caturi_dbg(hstr, src DBG_PARMS_INIT)
#define Str_cathtml(hstr, src)						Str_cathtml_dbg(hstr, src DBG_PARMS_INIT)

#define Str_destroy(hstr)							Str_destroy_dbg(hstr DBG_PARMS_INIT)
#define Str_tok(hres, ppch, delims)					Str_tok_dbg(hres,ppch,delims DBG_PARMS_INIT)
#define Str_ncpy(hstr, hsrc, N)						Str_ncpy_dbg(hstr,hsrc,n DBG_PARMS_INIT)
#define Str_rm(hstr, starting_offset, n_to_remove)	achr_rm_dbg(hstr,starting_offset,n_to_remove DBG_PARMS_INIT)
#define Str_setsize(hstr, n)						Str_setsize_dbg(hstr,n DBG_PARMS_INIT)
#define Str_cp_raw(hstr, src)						Str_cp_raw_dbg(hstr, src  DBG_PARMS_INIT)
#define Str_dup(src)								Str_dup_dbg(src DBG_PARMS_INIT)

char* Str_temp_dbg(void *mem, size_t mem_size DBG_PARMS);
void Str_clear_dbg(Str *hstr DBG_PARMS);
int Str_len_dbg(Str *hstr DBG_PARMS);
int Str_printf_dbg(Str *hstr DBG_PARMS, FORMAT format, ...);
int Str_vprintf_dbg(Str *hstr DBG_PARMS, FORMAT format, va_list args);
int Str_catf_dbg(Str *hstr DBG_PARMS, FORMAT format, ...);
int Str_vcatf_dbg(Str *hstr DBG_PARMS, FORMAT format, va_list args);

void Str_catprintable_dbg(Str *hstr, const char *src, char def DBG_PARMS);
void Str_caturi_dbg(Str *hstr, char *src DBG_PARMS);
void Str_cathtml_dbg(Str *hstr, char *src DBG_PARMS);

void Str_destroy_dbg(Str *hstr DBG_PARMS);
char *Str_tok_dbg(Str *hres, char const **ppch, char const *delims DBG_PARMS);
char *Str_ncpy_dbg(Str *hres, Str *src, int n DBG_PARMS);
void Str_remove_dbg(Str *hstr, int n, int offset DBG_PARMS);
void Str_setsize_dbg(Str *hstr, int n DBG_PARMS);               // not frequently used.
void Str_cp_raw_dbg(Str *hstr, const char *src DBG_PARMS);
char* Str_dup_dbg(const char *src DBG_PARMS);

ABHEADER_END
#endif // STR_H

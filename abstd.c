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
#include "abassert.h"

assert_static(sizeof(S32) == 4 && (S32)-1 == -1);



char *abstrdup(const char *str DBG_PARMS)
{
	char *temp;
	size_t n;
	if (!str) 
        return NULL;
    n = strlen(str)+1;
    temp = ABMALLOC_DBGCALL(n);
	strcpy_s(temp, n, str);
	return temp;
}

void* perftools_malloc(size_t size);
// static HashPtr global_alloc_tracker;
// static HashPtr cur_alloc_tracker;
// #define TRACK_ALLOCS 1
// static void track_alloc(void *p)
// {
// #if TRACK_ALLOCS
// #endif
// }

// static void untrack_alloc(void *p)
// {
// #if TRACK_ALLOCS
// #endif
// }

// void reset_cur_allocs()
// {
// }

// TrackedAlloc *get_cur_allocs()
// {
// }
void *abmalloc_dbg(size_t size DBG_PARMS)
{
#ifdef OVERRIDE_MALLOC
    void *tmp = perftools_malloc(size);
#else
    void *tmp = _malloc_dbg(size,_NORMAL_BLOCK,caller_fname,line);
#endif
    assert(tmp);
    caller_fname;
    line;
    return tmp;
}

void* perftools_calloc(size_t n, size_t elem_size);
void *abcalloc_dbg(size_t n, size_t s DBG_PARMS)
{
#ifdef OVERRIDE_MALLOC
    void *tmp = perftools_calloc(n,s);
#else
    void *tmp = _calloc_dbg(n,s,_NORMAL_BLOCK,caller_fname,line);
#endif
    assert(tmp);
    caller_fname;
    line;
    return tmp;    
}

void* perftools_realloc(void* old_ptr, size_t new_size);
void *abrealloc_dbg(void *p, size_t s DBG_PARMS)
{
#ifdef OVERRIDE_MALLOC
    void *tmp = perftools_realloc(p,s);
#else
    void *tmp = _realloc_dbg(p,s,_NORMAL_BLOCK,caller_fname,line);
#endif
    assert(tmp);
    caller_fname;
    line;
    return tmp;
}

void perftools_free(void *p);
void abfree_dbg(void *p DBG_PARMS)
{
    caller_fname;
    line;
#ifdef OVERRIDE_MALLOC
    perftools_free(p);
#else
    _free_dbg(p,_NORMAL_BLOCK);
#endif
}

char *strstr_advance(char **s,char const *str, int len)
{
    char *tmp;
    if(!s || !*s || !str)
        return NULL;
    tmp = strstr(*s,str);
    if(tmp)
    {
        tmp += len;
        *s = tmp;
    }
    return tmp;
}

char *strstr_advance_len(char **s,char const *str, int len)
{
    char *tmp;
    if(!s || !*s || !str)
        return NULL;
    tmp = strstr(*s,str);
    if(tmp)
    {
        tmp += len;
        *s = tmp;
    }
    return tmp;
}

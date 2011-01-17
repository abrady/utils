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
#include "errno.h"
#include "direct.h"     // mkdir
#include "time.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "ctype.h"
#include "Array.h"
#include "fileio.h"
#include "abhash.h"

// canonicalize_file_name : 
// The canonicalize_file_name function returns the absolute name of the file named by name which contains no ., .. components nor any repeated path separators (/) or symlinks. The result is passed back as the return value of the function in a block of memory allocated with malloc. If the result is not used anymore the memory should be freed with a call to free. 

//  Function: char * realpath (const char *restrict name, char *restrict resolved)

// A call to realpath where the resolved parameter is NULL behaves exactly like canonicalize_file_name. The function allocates a buffer for the file name and returns a pointer to it. If resolved is not NULL it points to a buffer into which the result is copied. It is the callers responsibility to allocate a buffer which is large enough. On systems which define PATH_MAX this means the buffer must be large enough for a pathname of this size. For systems without limitations on the pathname length the requirement cannot be met and programs should not call realpath with anything but NULL for the second parameter. 

// One other difference is that the buffer resolved (if nonzero) will contain the part of the path component which does not exist or is not readable if the function returns NULL and errno is set to EACCES or ENOENT.


//----------------------------------------
//  Our exes have an implicit 'root' directory
// that is different than the current working directory
// (maybe we should do this?)
//----------------------------------------
int path_isabsolute(const char *path)
{
    if(!path || !*path)
        return FALSE;

	if(	path[1]==':' ||
			(	path[0]=='.' &&
				!isalnum((unsigned char)path[1])) || // ./ .\\ .. .\0
			(	path[0]=='\\' &&
				path[1]=='\\') ||
			(	path[0]=='/' &&
				path[1]=='/')
		)
		return TRUE;
#ifdef _XBOX
	//Xbox has multi-letter drives
	{
		char *colon = strchr(path,':');
		char *bs = strchr(path,'\\');
		if (colon && (!bs || colon < bs))
		{
			// If the first colon is before the first slash, it's probably a drive
			return TRUE;
		}
	}
#endif
	return 0;
}

int file_exists(const char *fname)
{
	struct _stat32 status;
 
	if(!fname)
		return 0;

    if(!_stat32(fname, &status)){
        if(status.st_mode & _S_IFREG)
            return 1;
    }   
    return 0;
}

S64 file_size(char const *fname)
{
	struct _stat64 status;

	if(!fname)
		return 0;

    if(!_stat64(fname, &status)){ // _stat64 for big files...
        if(status.st_mode & _S_IFREG)
            return status.st_size;
    }   
    return 0;
}

// @todo -AB: get this called :11/03/08
void fileio_init(void)
{
    _setmaxstdio(2048); // CRT maximum?
}


// "r" Opens for reading. If the file does not exist or cannot be found, the fopen_s call fails. 
// "w" Opens an empty file for writing. If the given file exists, its contents are destroyed. 
// "a" Opens for writing at the end of the file (appending) without removing the EOF marker before writing new data to the file; creates the file first if it doesn't exist. 
// "r+" Opens for both reading and writing. (The file must exist.) 
// "w+" Opens an empty file for both reading and writing. If the given file exists, its contents are destroyed. 
// "a+" Opens for reading and appending; the appending operation includes the removal of the EOF marker before new data is written to the file and the EOF marker is restored after writing is complete; creates the file first if it doesn't exist.
//
// When a file is opened with the "a" or "a+" access type, all write
// operations occur at the end of the file. The file pointer can be
// repositioned using fseek or rewind, but is always moved back to the
// end of the file before any write operation is carried out. Thus,
// existing data cannot be overwritten.
//
// ab:
// - you probably always want to do "b" in the mode
// - win32 supports this funky css=UTF-8, how does that work with plain strings?
FILE *file_open_dbg(char const *fname, char *mode DBG_PARMS)
{
    FILE *fp;
    // track this...
    line;
    caller_fname;
    if(0 != fopen_s(&fp,fname,mode))
        return NULL;
    return fp;
}

void file_close_dbg(FILE *fp DBG_PARMS)
{
    // track this...
    line;
    caller_fname;
    fclose(fp);
}

void *file_alloc_dbg(char **hres, const char *fname DBG_PARMS)
{
    FILE	*fp;
    int total=0;
    intptr_t bytes_read;
	char	*mem=0;
    S64 filesize;

	if(!fname || (filesize = file_size(fname))<=0)
	{
		if(hres)
			achr_destroy(hres);
		return NULL;
	}

	if(filesize > INT_MAX)
	{
		if(hres)
			achr_destroy(hres);
		return NULL;
	}

	total = (int)filesize;
    fp = file_open_dbg(fname, "rb" DBG_PARMS_CALL);
	if(!fp)
	{
		if(hres)
			achr_destroy(hres);
		return NULL;
	}

    if(hres)
        mem = *hres;
    achr_setsize_dbg(&mem, total+1 DBG_PARMS_CALL);
    bytes_read = fread(mem,1,total+1,fp);
    assert(bytes_read==total);
    file_close_dbg(fp DBG_PARMS_CALL);
    mem[bytes_read] = 0;
    if(hres)
        *hres = mem;
	return mem;
}

#define PATH_SEPARATOR '/'
#define WRONG_PATH_SEPARATOR '\\'

char *fix_path_separators(char *path, size_t len)
{
	char	*s;

	if(!path)
		return NULL;

	for(s=path;*s;s++)
	{
		if (*s == WRONG_PATH_SEPARATOR)
			*s = PATH_SEPARATOR;
        // remove duplicates
		if (s!=path && (s-1)!=path && *s == PATH_SEPARATOR && *(s-1)==PATH_SEPARATOR)
			strcpy_safe(s,len-(s-path),s+1);
	}
	return path;

}

BOOL mkdirtree(const char *path_in)
{
    char path[MAX_PATH];
	char	*s;
	strcpy(path,path_in);
	fix_path_separators(ASTR(path));
	s = path;

	for(;;)
	{
        int err;
		s = strchr(s,PATH_SEPARATOR);
		if (!s)
			break;
		*s = 0;
        if(s-path == 2 && path[1] == ':')
        {
            // probably a drive spec, e.g. 'c:'
        }
        else
        {
            err = _mkdir(path);
            if(err && errno != EEXIST) 
            {
                WARN("couldn't make dir %s", path);
                return FALSE;
            }
        }
        
        
		*s = PATH_SEPARATOR;
		s++;
	} 
    return TRUE;
}

char* filename_nodir(char *dst, size_t n, const char *src)
{
    char *t;
    if(!src || !dst)
        return NULL;
    t = strrchr(src,PATH_SEPARATOR);
    if(!t)
        t = strrchr(src, WRONG_PATH_SEPARATOR);
    if(!t)
        strcpy_safe(dst,n,src);
    else
        strcpy_safe(dst,n,t+1);
    return dst;
}

char* dir_nofilename(char *dst, size_t n, const char *src)
{
    char *t;
    char path[MAX_PATH];
    if(!src || !dst)
        return NULL;
    strcpy(path,src);
    fix_path_separators(ASTR(path));
    t = strrchr(path,PATH_SEPARATOR);
    if(!t)
    {
        if(n<2)
            return NULL;
        dst[0] = '.';
        dst[1] = PATH_SEPARATOR;
        return dst;
    }
    t[1] = 0;
    strcpy_safe(dst,n,path);
    return dst;
}

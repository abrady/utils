/***************************************************************************
 *     Copyright (c) 2008-2008, Aaron Brady
 *     All Rights Reserved
 *     Confidential Property of Aaron Brady
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef FILEIO_H
#define FILEIO_H

#include "abstd.h"

ABHEADER_BEGIN

#define file_open(fname, mode) file_open_dbg(fname,mode DBG_PARMS_INIT)
#define file_close(fp) file_close_dbg(fp DBG_PARMS_CALL)
#define file_alloc(hres, fname) file_alloc_dbg(hres, fname DBG_PARMS_INIT) // return = hres = achr

void fileio_init(void);

FILE *file_open_dbg(char const *fname, char *mode DBG_PARMS);
void file_close_dbg(FILE *fp DBG_PARMS);
void *file_alloc_dbg(char **hres, const char *fname DBG_PARMS);
int file_exists(const char *fname);
S64 file_size(char const *fname);

// *************************************************************************
//  directory manipulation
// *************************************************************************

#define PATH_SEPARATOR '/'
#define WRONG_PATH_SEPARATOR '\\'

char *fix_path_separators(char *path, size_t len);
BOOL mkdirtree(const char *path_in);
char* filename_nodir(char *dst, size_t n, const char *src);
char* dir_nofilename(char *dst, size_t n, const char *src);


#if WIN32
#define fopen_safe fopen_s
#endif

ABHEADER_END
#endif //FILEIO_H

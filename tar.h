/***************************************************************************
 *     Copyright (c) 2008-2008, Aaron Brady
 *     All Rights Reserved
 *     Confidential Property of Aaron Brady
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef TAR_H
#define TAR_H

typedef struct TarHdr
{
    char fname[100];
    U32 file_size;
    U32 last_mod_time;    
} TarHdr;

typedef struct Tar
{
    U8 *start;
    U8 *data;
    U32 len;
    TarHdr hdr;
} Tar;

BOOL tar_process_cur(Tar *t);
void *tar_cur_file(Tar *t, U32 *len);
void *tar_alloc_cur_file(Tar *t, U32 *len); // must free 
BOOL tar_next(Tar *t);

#endif //TAR_H

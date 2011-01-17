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
#include <stdio.h>
#include <sys/stat.h>


#define TAR_HDR_SIZE 512


Tar *tar_from_data(U8 *data, U32 len)
{
    Tar *t;
    if(!data)
        return NULL;
    t = (Tar*)calloc(sizeof(Tar),1);
    t->start = data;
    t->data = data;
    t->len = len;
    return t;
}

void Tar_Destroy(Tar *t)
{
    free(t);
}

// only grabs the filename, file size.
BOOL tar_process_cur(Tar *t)
{
    char *data;
    if(!t)
        return FALSE;
    data = t->data;
    strncpy(t->hdr.fname,data,100);
    data+=100;
    data+=24; // skip mode, owner and user id
    if(1!=sscanf(data,"%o",&t->hdr.file_size))
        return FALSE;
    
    data = t->data + 257;
    if(data[0] != 'u' || data[1] != 's' || data[2] != 't' || data[3] != 'a' || data[4] != 'r')
        return FALSE;
    return TRUE;
}

void *tar_cur_file(Tar *t, U32 *len)
{
    if(!tar_process_cur(t))
        return NULL;
    if(len)
        *len = t->hdr.file_size;
    return t->data + TAR_HDR_SIZE;
}

// must free returned value
void *tar_alloc_cur_file(Tar *t, U32 *len)
{
    char *s;
    char *d;
    int n;
    
    if(!tar_process_cur(t))
        return NULL;
    n = t->hdr.file_size;
    if(len)
        *len = n;
    d = (char*)malloc(n);
    s = t->data + TAR_HDR_SIZE;
    memcpy(d,s,n);
    return d;
}

BOOL tar_next(Tar *t)
{
    if(!tar_process_cur(t))
        return FALSE;
    t->data += ((TAR_HDR_SIZE + t->hdr.file_size + 511) & ~511);
    if(t->data >= t->start + t->len)
        return FALSE;
    return tar_process_cur(t);
}

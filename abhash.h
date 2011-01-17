/***************************************************************************
 *     Copyright (c) 2008-2008, Aaron Brady
 *     All Rights Reserved
 *     Confidential Property of Aaron Brady
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef abhash_H
#define abhash_H

#include "abstd.h"
ABHEADER_BEGIN

typedef struct HashStr HashStr;
typedef struct HashPtr HashPtr;

HashStr* HashStr_Create(int size, BOOL copy_keys);
void HashStr_Destroy(HashStr **ht);
void *HashStr_Find(HashStr **ht, char const *key);
void *HashStr_Insert(HashStr **ht, char const *key, void *value, BOOL overwrite);
BOOL HashStr_Remove(HashStr **ht, char const *key);


HashPtr* HashPtr_Create(int size);
void HashPtr_Destroy(HashPtr **ht);
void *HashPtr_Find(HashPtr **ht, void const *key);
void *HashPtr_Insert(HashPtr **ht, void const *key, void *value, BOOL overwrite);
BOOL HashPtr_Remove(HashPtr **ht,void const *key);


ABHEADER_END
#endif //abhash_H

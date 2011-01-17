/***************************************************************************
 *     Copyright (c) 2008-2008, Aaron Brady
 *     All Rights Reserved
 *     Confidential Property of Aaron Brady
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifdef UTILITIESLIB
#	include "file.h"
#	undef toupper
#	undef FILE
#endif

#include "abstd.h"
#include "abhash.h"
#pragma warning(push)
#pragma warning(disable:6308)
#include <google/dense_hash_map>
#pragma warning(pop)

#pragma warning(disable:4312) // cast from int to ptr
#define DELETED_VAL ((void*)0xDE7EE7ED)

// functor for string hash
// have to check for deleted value
struct strcmp_fnc {
  bool operator()(const char* s1, const char* s2) const {
      if(s1 == DELETED_VAL)
          return s2 == DELETED_VAL;
      else if(s2 == DELETED_VAL)
          return FALSE;
      return ((s1 == 0 && s2 == 0) ||
              (s1 && s2 && *s1 == *s2 && strcmp(s1, s2) == 0));
  }
};

extern "C" U32 burtle_hashstr( const char *str );
struct strhash_fnc 
{
    size_t operator()(const char *s) const {
        return burtle_hashstr(s);
    }
};
    

typedef struct HashStr
{
    HashStr(int size, BOOL copy_keys) : ht(size), copy_keys(copy_keys) {
        ht.set_empty_key(NULL);
        ht.set_deleted_key((const char *)DELETED_VAL);
    }
    ~HashStr() {
		if(copy_keys)
		{
			for(strhash::iterator it = ht.begin(); it != ht.end(); ++it)
			{
				char const *key = it->first;
				free((void*)key);
			}
		}
    }

	typedef google::dense_hash_map<char const*, void*, strhash_fnc, strcmp_fnc> strhash;
	strhash	ht;
    BOOL copy_keys;
} HashStr;

ABFUNC HashStr* HashStr_Create(int size, BOOL copy_keys)
{
    return new HashStr(size,copy_keys);
}

ABFUNC void HashStr_Destroy(HashStr **ht)
{
    if(ht)
    {
        delete *ht;
        *ht = NULL;
    }
}

ABFUNC void *HashStr_Find(HashStr **hht, char const *key)
{
    if(!hht || !*hht)
        return NULL;
    HashStr *&ht = *hht;
	HashStr::strhash::iterator it = ht->ht.find(key);
    if(it != ht->ht.end())
        return it->second;
    return NULL;
}

ABFUNC void *HashStr_Insert(HashStr **hht, char const *key, void *value, BOOL overwrite)
{
    if(!hht)
        return NULL;
    if(!key)
        return NULL;
    HashStr *&ht = *hht;
    if(!ht)
        ht = HashStr_Create(0,TRUE); // create with deep copy
    
	HashStr::strhash::iterator it = ht->ht.find(key);
    if(it != ht->ht.end())
	{
		if(!overwrite)
			return NULL;
		else
			return it->second = value;		
	}
	else // not already existing
	{
		if(ht->copy_keys) // only deep copy key if not found
			key = _strdup(key);
		return ht->ht[key] = value;
	}
}

ABFUNC BOOL HashStr_Remove(HashStr **hht, char const *key)
{
    if(!key)
        return FALSE;
    if(!hht || !*hht)
        return FALSE;
    HashStr *&ht = *hht;
    
	HashStr::strhash::iterator it = ht->ht.find(key);
    if(it != ht->ht.end())
    {
        if(ht->copy_keys)
            free((void*)it->first);
        ht->ht.erase(it);
        return TRUE;
    }
    return FALSE;
}

    

// *************************************************************************
// HashPtr
// *************************************************************************

// functor for string hash
struct ptrcmp_fnc {
  bool operator()(const void* s1, const void* s2) const {
      return s1 == s2;
  }
};

struct ptrhash_fnc 
{
    size_t operator()(const void *p) const {
        return (size_t)p;
    }
};

typedef struct HashPtr
{
    HashPtr(int size) : ht(size) {
        ht.set_empty_key((void*)(intptr_t)0xFBAD4A55); // bad hash :P
        ht.set_deleted_key((const char *)DELETED_VAL);

    }
	typedef google::dense_hash_map<void const*, void*, ptrhash_fnc, ptrcmp_fnc> hash;
	hash	ht;
} HashPtr;

ABFUNC HashPtr* HashPtr_Create(int size)
{
    return new HashPtr(size);
}

ABFUNC void HashPtr_Destroy(HashPtr **ht)
{
    if(ht)
    {
        delete *ht;
        *ht = NULL;
    }
}

ABFUNC void *HashPtr_Find(HashPtr **hht, void const *key)
{
    if(!hht || !*hht)
        return NULL;
    HashPtr *&ht = *hht;
	HashPtr::hash::iterator it = ht->ht.find(key);
    if(it != ht->ht.end())
        return it->second;
    return NULL;
}

ABFUNC void *HashPtr_Insert(HashPtr **hht,void const *key, void *value, BOOL overwrite)
{
    if(!key || !hht)
        return NULL;
    HashPtr *&ht = *hht;
    if(!ht)
        ht = HashPtr_Create(0);
	HashPtr::hash::iterator it = ht->ht.find(key);
    if(it != ht->ht.end())
	{
		if(!overwrite)
			return NULL;
		else
			return it->second = value;		
	}
	else // not already existing
	{
		return ht->ht[key] = value;
	}
}

ABFUNC BOOL HashPtr_Remove(HashPtr **hht, void const *key)
{
    if(!key || !hht || !*hht)
        return NULL;
    return !!(*hht)->ht.erase(key);
}

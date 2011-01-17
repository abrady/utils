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
#include "Array.h"
#include "abhash.h"


#define TYPE_T void*
#define TYPE_FUNC_PREFIX ap
#include "array_def.c"
#undef TYPE_T
#undef TYPE_FUNC_PREFIX

#define TYPE_T char
#define TYPE_FUNC_PREFIX achr
#include "array_def.c"
#undef TYPE_T
#undef TYPE_FUNC_PREFIX

#define TYPE_T int
#define TYPE_FUNC_PREFIX aint
#include "array_def.c"
#undef TYPE_T
#undef TYPE_FUNC_PREFIX

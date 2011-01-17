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
#include "abunittest.h"
#include "abhash.h"

extern "C" void abmem_unittest(void)
{
    TEST_START("strdup");
    {
        int i;
        char *strs[1024];
        for( i = 0; i < ARRAY_SIZE(strs); ++i ) 
        {
            char tmp[128];
            sprintf_s(ASTR(tmp),"%i",i);
            strs[i] = _strdup(tmp);
            free(strs[i]);
        }
    }
    TEST_END;
}

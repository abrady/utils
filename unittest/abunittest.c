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

TestSuite g_testsuite;

void abmem_unittest(void);

extern void abhash_unittest(void);
extern void abstd_unittest();
extern int lookup3_unittest();
extern void abarray_unittest(void);
extern void abarray_unittest(void);
extern void abarray_unittest(void);
    extern void str_unittest(void);
    extern void abhttp_unittest(void);
    extern void net_unittest(void);
    extern void stream_unittest(void);
void abunittest_run(void)
{
    // tests
    {
        abstd_unittest();
        abmem_unittest();
        lookup3_unittest(); // string hash function
        abhash_unittest();
        abarray_unittest();
        str_unittest();
        stream_unittest();
        net_unittest();
        abhttp_unittest();
    }
    printf("unit tests done.\n");
}

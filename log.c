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
#include "array.h"
#include "log.h"
#include "abhash.h"

static int *s_log_level_stack;
LogLevel g_log_level; // >= means display this

void loglevel_push(LogLevel l)
{
    aint_push(&s_log_level_stack,g_log_level); 
    g_log_level = l;
}

void loglevel_pop()
{
    g_log_level = *aint_pop(&s_log_level_stack);
}



int logvpf(LogLevel lvl, char *system, FORMAT format, va_list args)
{
    int n;
    if(lvl < g_log_level)
        return 0;
    if(system)
        printf("[%s]",system);
    n = vprintf(format,args);
    printf("\n");
    return n;
}

int logpf(LogLevel lvl, char *system, FORMAT format, ...)
{
    int r;
    // todo: print to the generic log file...
    // really just figure out a logging strategy
    VA_START(args, format);
    r = logvpf(lvl,system,format,args);
    VA_END();
    return r;
}

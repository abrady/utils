/***************************************************************************
 *     Copyright (c) 2008-2008, Aaron Brady
 *     All Rights Reserved
 *     Confidential Property of Aaron Brady
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#ifndef LOG_H
#define LOG_H

#include "abstd.h"

ABHEADER_BEGIN

typedef enum LogLevel
{
    LogLevel_Info,
    LogLevel_Med,
    LogLevel_High,
    LogLevel_Count
} LogLevel;

extern LogLevel g_log_level; // >= means display this

void loglevel_push(LogLevel l);
void loglevel_pop();

int logpf(LogLevel lvl, char *system, FORMAT format, ...);

#define LOG_SYSTEM NULL // default, undef and define
#define LOG(FMT,...) logpf(LogLevel_Info,LOG_SYSTEM,FMT,__VA_ARGS__)
#define WARN(FMT,...) logpf(LogLevel_Med,LOG_SYSTEM,FMT,__VA_ARGS__)
#define ERR(FMT,...) logpf(LogLevel_High,LOG_SYSTEM,FMT,__VA_ARGS__)  // @todo -AB: popup for this :09/24/08
ABHEADER_END
#endif //LOG_H

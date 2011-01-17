/***************************************************************************
 *     Copyright (c) 2008-2008, Aaron Brady
 *     All Rights Reserved
 *     Confidential Property of Aaron Brady
 ***************************************************************************/
#pragma once
#include "abstd.h"
ABHEADER_BEGIN

// Interface ///////////////////////////////////////////////////////////////////
// LazyLocks are safe to use from any thread and don't require initialization

typedef struct LazyLock LazyLock;

ABINLINE void lazyLock(LazyLock *lock);		// waits indefinitely for the lock
ABINLINE int lazyLockTry(LazyLock *lock);	// doesn't wait, returns success
ABINLINE void lazyUnlock(LazyLock *lock);	// releases the lock

////////////////////////////////////////////////////////////////////////////////

#include <WinBase.h>

struct LazyLock
{
	S32 init;
	CRITICAL_SECTION cs;
};

void lazyLockInit(volatile S32 *init, CRITICAL_SECTION *cs);

ABINLINE void lazyLock(LazyLock *lock)
{
	if(!lock->init)
		lazyLockInit(&lock->init, &lock->cs);
	EnterCriticalSection(&lock->cs);
}

ABINLINE int lazyLockTry(LazyLock *lock)
{
	if(!lock->init)
		lazyLockInit(&lock->init, &lock->cs);
	return !!TryEnterCriticalSection(&lock->cs);
}

ABINLINE void lazyUnlock(LazyLock *lock)
{
	LeaveCriticalSection(&lock->cs);
}

ABHEADER_END

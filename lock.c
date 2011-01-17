#include "lock.h"

void lazyLockInit(volatile S32 *init, CRITICAL_SECTION *cs)
{
	char name[MAX_PATH];
	HANDLE hMutex;
	sprintf(name, "LazyLockInit:%d:%x", _getpid(), init); // addr of init should be the address of the lock

	hMutex = CreateMutex(NULL, FALSE, name);
	assertmsg(hMutex, "Can't create mutex.");

	switch(WaitForSingleObject(hMutex, INFINITE))
	{
		xcase WAIT_OBJECT_0:
		xcase WAIT_ABANDONED:

		xcase WAIT_TIMEOUT:		assert(0); // these should never happen
		xcase WAIT_FAILED:		assert(0);
		xdefault:				assert(0);
	}

	if(!*init)
	{
		InitializeCriticalSection(cs);
		*init = 1;
	}

	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
}

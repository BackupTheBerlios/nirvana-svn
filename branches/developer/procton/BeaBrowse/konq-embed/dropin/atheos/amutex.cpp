#include <assert.h>
#include <unistd.h>

#include <Application.h>
#include <Bitmap.h>
#include <List.h>
#include <OS.h>
#include <Locker.h>
#include <Autolock.h>
#include <Looper.h>
#include "amutex.h"

#include <stdio.h>
#include <stack>

#include <kdebug.h>

BLocker  g_cLock( "khtml_gmutex" );
//static os::Looper* g_pcCurLooper = NULL;
//static std::stack<BLooper*>	g_cLooperStack;

void GlobalMutex::Lock()
{
	g_cLock.Lock();
}

void GlobalMutex::Unlock()
{
//	assert( g_cLock.LockingThread() == getpid() );
//	assert( g_cLock.LockingThread() == GetCurLooper()->Thread() );
//	assert( g_cLock.LockingThread() == find_thread(NULL) );
	assert( g_cLock.IsLocked() );
	g_cLock.Unlock();
}

BLocker* GlobalMutex::GetMutex()
{
	return( &g_cLock );
}
#if 0
void GlobalMutex::PushLooper( BLooper* pcLooper )
{
	g_cLooperStack.push( pcLooper );
//	g_pcCurLooper = pcLooper;
}

void GlobalMutex::PopLooper()
{
	g_cLooperStack.pop();
}

BLooper* GlobalMutex::GetCurLooper()
{
	return( g_cLooperStack.top() );
}
#endif


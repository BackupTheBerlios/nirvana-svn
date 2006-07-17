#ifndef __F_ATHEOS_AMUTEX_H__
#define __F_ATHEOS_AMUTEX_H__

class BLooper;
class BLocker;
class BBitmap;

class GlobalMutex
{
public:
	static void Lock();
	static void Unlock();
	static BLocker* GetMutex();
	
#if 0
	static void PushLooper( BLooper* pcLooper );
	static void PopLooper();
	static BLooper* GetCurLooper();	
#endif

};




#endif // __F_ATHEOS_AMUTEX_H__

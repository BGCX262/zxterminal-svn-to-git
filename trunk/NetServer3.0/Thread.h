

///////////////////////////////////////////////////////////////////////
#ifndef _THREAD_H_
#define _THREAD_H_

#include <windows.h>

class Thread
{
public:	
	Thread();						/* Default construction,DETACHED */
	~Thread();	
	virtual int Start();			/*start the thread with Run()*/
	virtual int Stop();				/*Force to terminate the thread*/
	DWORD GetThreadId();
	DWORD m_dwThreadId;
	HANDLE  m_hThread;

protected:
	BOOL m_bStopThread;

private:
	virtual void Run()=0;			/*Execbute function of the thread*/	
	static void __cdecl ThreadMain(void* lpParameter);
};

#endif


///////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <stdio.h>
//	reference to thread
#include "process.h"
#include "Thread.h"

Thread::Thread()
{
	m_dwThreadId = 0;
	m_bStopThread = FALSE;
}

Thread::~Thread()
{
}

int Thread::Start()
{
	m_bStopThread = FALSE;
	m_hThread = (HANDLE)::_beginthread(ThreadMain, 0, static_cast<void*>(this));
    if(FAILED(m_hThread))
	     return 0;
	else
		return 1;
} 

void __cdecl Thread::ThreadMain(void* lpParameter)
{
	Thread* pThis = (Thread *)lpParameter;
	// Start execute function 
	pThis->Run();
}

int Thread::Stop()
{
	m_bStopThread = TRUE;	
	return 0;
}

DWORD Thread::GetThreadId()
{
	return m_dwThreadId;
}

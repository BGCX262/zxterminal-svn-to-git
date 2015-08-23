// SocketService.cpp: implementation of the CSocketService class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SocketService.h"
#include "Net.h"

#include "MemoryPool.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern  int CreateTcpSocketServer(const  char  *szip,const short iPort);
extern  void ReleaseSocket(int  fd);

CMutex    g_MemPoolMutexLock(FALSE,"MemPoolMutexLock");
CMemoryPool   g_MemPool;  //用的一个线程池;

CSocketService::CSocketService()
{
	

}

CSocketService::~CSocketService()
{

}

BOOL CSocketService::StartService()
{
	CSystemInfo   systenifo;
	short  iPort=systenifo.GetPort();
	fd=CreateTcpSocketServer(NULL,iPort);
	if(fd>0)
	{ 

        BOOL bR=m_ReadThread.Start(fd);
	    BOOL bW=m_WorkThread.Start();
		if(bR &&  bW)
			return  TRUE;
	}
	return  FALSE;
}


int CSocketService::EndService()
{
	ReleaseSocket(fd);
	m_ReadThread.Stop();
	m_WorkThread.Stop();

	g_MemPool.ReleasePool();

	return  1;
}





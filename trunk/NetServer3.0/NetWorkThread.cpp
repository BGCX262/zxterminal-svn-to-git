// NetWorkThread.cpp: implementation of the CNetWorkThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetServer.h"
#include "NetWorkThread.h"
#include "NetField.h"
#include "ThreadMsgManager.h"
#include "NetAgreement.h"
#include "MemoryPool.h"
#include "Log.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern  void      g_Log(CString szLog);
extern  CThreadMsgManager  g_ThreadMsgManager;
//处理消息的全局函数;
/*/
/**/
/**/
extern  CMutex    g_MemPoolMutexLock;
extern  CMemoryPool   g_MemPool;  //用的一个线程池;

UINT g_DealWithMsg(void* pParmer)
{
	if(!pParmer) 
	{
		return  -1;	
	}

	ThreadMsg* pMsg = ((ThreadMsg *)pParmer);	

	char* pBuf = pMsg->pBuf;
	int fd = pMsg->fd;
	int iLen = pMsg->iLen;

	delete (ThreadMsg *)pParmer;   

	//以下是处理相关的请求
	NetMsgHead* p = (NetMsgHead*)pBuf;
	p->ContentLen = ntohl(p->ContentLen);
	p->MainMsgType = (MIANMSGTYPE)ntohl(p->MainMsgType);
	p->SubMsgType = (SUBMSGTYPE)ntohl(p->SubMsgType);
	p->NetMsgLen = ntohl(p->NetMsgLen);
	p->Version = ntohl(p->Version);

	//版本号的比较;
	if(p->Version != VERSION) 
	{
		//版本号不正确; 回应信息：版本太低  //暂时不用
		//return 0;
	}	

	CNetAgreement* pNetAgreement = new CNetAgreement(fd, p, iLen);  //这里写的有问题，不应该总是在创建;
	pNetAgreement->DealNetMsg();

	g_MemPoolMutexLock.Lock();
	g_MemPool.ReleaseBuf((char*)p);	
	g_MemPoolMutexLock.Unlock();

	delete pNetAgreement;
	pNetAgreement = NULL;

	//delete pMsg;
	//pMsg = NULL;

	return  0;
}

CNetWorkThread::CNetWorkThread()
{
	m_bRun=TRUE;

}

CNetWorkThread::~CNetWorkThread()
{

}



void CNetWorkThread::Run()
{
	CString  szTempBegin="CNetWorkThread::Run  Begin";
	g_Log(szTempBegin);	//主要工作函数;


	while (!this->m_bStopThread)
	{

		ThreadMsg  *pMsg= new  ThreadMsg ;
		if(!pMsg)
			continue;
		if(!this->GetNetMsg(*pMsg))
		{
			//当前没有可以用的消息包;
			Sleep(1000);

			delete pMsg;
			pMsg = NULL;

			continue;
		}
		//得到信息后的处理;生成一个新的线程来处理;  以后这里可以处理成线程池,一处理大量用户的请求;
		AfxBeginThread(g_DealWithMsg,(void *)pMsg);//??好象有点问题；
	}

	CString  szTempend="CNetWorkThread::Run  end";
	g_Log(szTempend);	//主要工作函数;
}

BOOL CNetWorkThread::GetNetMsg(ThreadMsg   &Msg)
{
	if(!g_ThreadMsgManager.GetTheadMsg(Msg))
		return  FALSE;
	return  TRUE;
}

void CNetWorkThread::ReleaseBuf(CNetNode *pNetData)
{
	g_MemPoolMutexLock.Lock();
	g_MemPool.ReleaseBuf(pNetData->pBuf);
	pNetData->pBuf=NULL;
	g_MemPoolMutexLock.Unlock();

}

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
//������Ϣ��ȫ�ֺ���;
/*/
/**/
/**/
extern  CMutex    g_MemPoolMutexLock;
extern  CMemoryPool   g_MemPool;  //�õ�һ���̳߳�;

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

	//�����Ǵ�����ص�����
	NetMsgHead* p = (NetMsgHead*)pBuf;
	p->ContentLen = ntohl(p->ContentLen);
	p->MainMsgType = (MIANMSGTYPE)ntohl(p->MainMsgType);
	p->SubMsgType = (SUBMSGTYPE)ntohl(p->SubMsgType);
	p->NetMsgLen = ntohl(p->NetMsgLen);
	p->Version = ntohl(p->Version);

	//�汾�ŵıȽ�;
	if(p->Version != VERSION) 
	{
		//�汾�Ų���ȷ; ��Ӧ��Ϣ���汾̫��  //��ʱ����
		//return 0;
	}	

	CNetAgreement* pNetAgreement = new CNetAgreement(fd, p, iLen);  //����д�������⣬��Ӧ�������ڴ���;
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
	g_Log(szTempBegin);	//��Ҫ��������;


	while (!this->m_bStopThread)
	{

		ThreadMsg  *pMsg= new  ThreadMsg ;
		if(!pMsg)
			continue;
		if(!this->GetNetMsg(*pMsg))
		{
			//��ǰû�п����õ���Ϣ��;
			Sleep(1000);

			delete pMsg;
			pMsg = NULL;

			continue;
		}
		//�õ���Ϣ��Ĵ���;����һ���µ��߳�������;  �Ժ�������Դ�����̳߳�,һ��������û�������;
		AfxBeginThread(g_DealWithMsg,(void *)pMsg);//??�����е����⣻
	}

	CString  szTempend="CNetWorkThread::Run  end";
	g_Log(szTempend);	//��Ҫ��������;
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

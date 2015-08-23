// ThreadMsgManager.cpp: implementation of the CThreadMsgManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetServer.h"
#include "ThreadMsgManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//定义
CThreadMsgManager  g_ThreadMsgManager;


CThreadMsgManager::CThreadMsgManager()
{

	this->m_asThreadMsg.SetSize(0);
}

CThreadMsgManager::~CThreadMsgManager()
{

}

//add  threadmsg;
void CThreadMsgManager::Add(ThreadMsg  thMsg)
{
	m_Lock.Lock();
	m_asThreadMsg.Add(thMsg);
	m_Lock.Unlock();

}

//  get   and  del
BOOL CThreadMsgManager::GetTheadMsg(ThreadMsg &thMsg)
{
	m_Lock.Lock();
	if(m_asThreadMsg.GetSize()>0)
	{
		thMsg=m_asThreadMsg.GetAt(0);
		m_asThreadMsg.RemoveAt(0);
		m_Lock.Unlock();
		return  TRUE;
		
	}
	m_Lock.Unlock();
	return  FALSE;
}

//清除所有的消息;
void CThreadMsgManager::Clear()
{

	m_Lock.Lock();
	for(int  i=0;i<m_asThreadMsg.GetSize();i++)
	{
		ThreadMsg  temp=m_asThreadMsg.GetAt(0);
		char  * p=temp.pBuf;
		delete []p;		
	}
	this->m_asThreadMsg.RemoveAll();
	this->m_asThreadMsg.SetSize(0);
	m_Lock.Unlock();
}

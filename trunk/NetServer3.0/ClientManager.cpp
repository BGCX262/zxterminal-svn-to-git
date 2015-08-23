// ClientManager.cpp: implementation of the CClientManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetServer.h"
#include "ClientManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern  CNetServerApp  theApp;
CMutex  g_ClientManagerLock(FALSE,"ClientManagerLock");
CClientManager::CClientManager()
{
	m_aClientArray.RemoveAll();
	m_aClientArray.SetSize(0);

}

CClientManager::~CClientManager()
{

}

BOOL CClientManager::AddNode(int fd, short iPort,CString szIP,CString  szUserId)
{
	int index=FindNode(fd);
	if(index==-1)
	{
		ClintNode  *pNode=new ClintNode;
		if(!pNode)
			return  FALSE;
		pNode->fd=fd;
		if(iPort!=0)
		{			
			pNode->addr.sin_port=ntohs(iPort);
			pNode->addr.sin_addr.s_addr=inet_addr(szIP.GetBuffer(0));
		}

		if(!szUserId.IsEmpty())
		{
			sprintf(pNode->UserID,"%s",szUserId);
			pNode->UserID[szUserId.GetLength()]=0;
		}
//		m_Lock.Lock();
		m_aClientArray.Add((CObject *)pNode);
//		m_Lock.Unlock();
	}
	else
	{
//		m_Lock.Lock();
		ClintNode  *pNode=(ClintNode  *)m_aClientArray[index];
		
		if(iPort!=0)
		{			
			pNode->addr.sin_port=ntohs(iPort);
			pNode->addr.sin_addr.s_addr=inet_addr(szIP.GetBuffer(0));
		}
		if(!szUserId.IsEmpty())
		{
			sprintf(pNode->UserID,"%s",szUserId);
			pNode->UserID[szUserId.GetLength()]=0;
		}
				
//		m_Lock.Unlock();
	}
	return  TRUE;
}



int CClientManager::FindNode(int fd)
{
//	m_Lock.Lock();
	int iCount=m_aClientArray.GetSize();
	for(int i=0;i<iCount;i++)
	{
		ClintNode  *pNode=(ClintNode  *)m_aClientArray[i];
		if(!pNode)
			continue;
		if(fd==pNode->fd)
		{
//			m_Lock.Unlock();
			return  i;
		}
	}
//	m_Lock.Unlock();
	return  -1;
}

BOOL CClientManager::DeletNode(int fd)
{
//	m_Lock.Lock();
	
	int iCount=m_aClientArray.GetSize();
	int index=FindNode(fd);
	if(index==-1)
	{
//		m_Lock.Unlock();
		return FALSE;
	}
	else
	{
		ClintNode  *pNode=(ClintNode  *)m_aClientArray[index];
		delete  pNode;
		m_aClientArray.RemoveAt(index);		
	}
//	m_Lock.Unlock();
				
	return  TRUE;

}
int CClientManager::GetUserCount()
{
	return   m_aClientArray.GetSize();
	
}
ClintNode  * CClientManager::GetClintNode(int fd)
{
	
	int  index=FindNode(fd);
	if(index==-1)
		return  NULL;
	ClintNode  *pNode=(ClintNode  *)m_aClientArray[index];
	if(pNode==NULL)
		return NULL;
	return   pNode;
	
}

void CClientManager::GetInfo(int fd,CString &szUserId, CString  &szIP, CString  &szTime)
{
	int index=FindNode(fd);
	if(index!=-1)
	{
		
		ClintNode  *pNode=(ClintNode  *)m_aClientArray[index];
		if(pNode)
		{
			szUserId=pNode->UserID;
			szIP=inet_ntoa(pNode->addr.sin_addr);
			szTime=pNode->szLoginTime;
		}		
	}

}

void CClientManager::GetClientofFd(const CString &szUser, int &fd)
{
	
	
	int iCount=m_aClientArray.GetSize();
	for(int i=0;i<iCount;i++)
	{

		ClintNode  *pNode=(ClintNode  *)m_aClientArray[i];
		if(!pNode)
			continue;
		if(szUser==pNode->UserID)
		{
			fd=pNode->fd;
			return;
		}
	}
	fd=INVALID_SOCKET;
}
ClintNode  * CClientManager::GetClientNodeFromId(int index)
{
	if(index<m_aClientArray.GetSize())
	{
		ClintNode  *pNode=(ClintNode  *)m_aClientArray[index];
		return   pNode;
		
		
	}
	return  NULL;
	
}


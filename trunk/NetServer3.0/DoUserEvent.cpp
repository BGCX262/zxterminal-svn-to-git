// DoUserEvent.cpp: implementation of the CDoUserEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DoUserEvent.h"
#include "NetServer.h"
#include "NetField.h"
#include "DBWork.h"
#include "CommField.h"
#include "SystemInfo.h"
#include "Winsock.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define   MTGPSMAXCOMMANDID  30
extern  CNetServerApp  theApp;
extern  CMutex  g_ClientManagerLock;
extern  CDBWork   g_DBWork;
CDoUserEvent::CDoUserEvent()
{

}

CDoUserEvent::CDoUserEvent(int iSocketIndex,CString  szEquimentID,int iCommandId,CString  szParma,int BSuccess)
{
	m_szEquimentID=szEquimentID;
	m_iCommandId=iCommandId;
	m_szParma=szParma;
	m_iSocketIndex=iSocketIndex;
	m_BeOK=BSuccess;
	
}

CDoUserEvent::~CDoUserEvent()
{

}

BOOL CDoUserEvent::GetUserId(int iSocketIndex,CString  &szuserId)
{

	g_ClientManagerLock.Lock();	
	int  iCount=theApp.m_Clientmanager.GetUserCount();
	for(int  iLoop=0;iLoop<iCount;iLoop++)
	{
		ClintNode *pNode=theApp.m_Clientmanager.GetClientNodeFromId(iLoop);
		if(pNode &&pNode->fd==iSocketIndex)
		{
			szuserId=pNode->UserID;
			m_UserIP=inet_ntoa(pNode->addr.sin_addr);
			g_ClientManagerLock.Unlock();
			return  TRUE;
		}
	}
	g_ClientManagerLock.Unlock();
	return  FALSE;
}

BOOL CDoUserEvent::GetEuqimentCommandText(CString szEuqimentId,int iCoomandId,CString &szCommandText)
{

	szEuqimentId.TrimLeft(),szEuqimentId.TrimRight();
	CEquimentInfo  info;
    if(g_DBWork.GetEquimentInfo(szEuqimentId,info)<0)
		return FALSE;
	int  iType=info.EquimentSubType;
	if(iType==EQUIPMENT_MTGPS)
	{//MTGPS���豸����;
		if(iCoomandId<0  ||  iCoomandId  >=MTGPSMAXCOMMANDID)
			return  FALSE;
		CString  szCommandinfo[30]={"�趨����","�������","�������","����","�������",
			"�������","����յ�","��������","����β��","��ǰ״̬","��ѯ����","ʵʱ���","ֹͣ���",
			"����ң��","����ң��","������","�ؼ���","ѧϰң��","����ң��","����","���Ի�","�ضԻ�",
			"������һ","�����ȶ�","��������","����Ϩ��","Ϩ��ָ�","�������","����","�������"};
		szCommandText=szCommandinfo[iCoomandId];	
	}
	else
		return  FALSE;
	return  TRUE;
}


BOOL CDoUserEvent::RecordUserEvent()
{
	//InsertUserevent
	CString  UserId,szText;
	if( GetUserId(m_iSocketIndex,UserId)  && GetEuqimentCommandText(m_szEquimentID,m_iCommandId,szText))
	{

		m_szParma.TrimLeft(),m_szParma.TrimRight();
		g_DBWork.InsertUserevent(UserId,szText,m_szEquimentID,m_szParma,m_UserIP,m_BeOK);
		return  TRUE;
	}
	return    FALSE;

}

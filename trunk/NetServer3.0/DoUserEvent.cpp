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
	{//MTGPS的设备命令;
		if(iCoomandId<0  ||  iCoomandId  >=MTGPSMAXCOMMANDID)
			return  FALSE;
		CString  szCommandinfo[30]={"设定防盗","静音设防","部分设防","防抢","防抢解除",
			"解除防盗","求救收到","启动报警","开车尾箱","当前状态","查询车辆","实时监控","停止监控",
			"启动遥控","屏蔽遥控","开监听","关监听","学习遥控","消除遥控","设置","开对话","关对话",
			"灵敏度一","灵敏度二","灵敏度三","断油熄火","熄火恢复","紧急解除","锁车","解除锁车"};
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

// GmemoryDataManager.cpp: implementation of the CGmemoryDataManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DBWork.h"

#include "GmemoryDataManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern CDBWork   g_DBWork;

CGmemoryDataManager::CGmemoryDataManager()
{

}

CGmemoryDataManager::~CGmemoryDataManager()
{
	for (int i = 0; i < m_aEquimentUser.GetCount(); i ++)
	{
		CEquimentUser *peuser = (CEquimentUser *)m_aEquimentUser.GetAt(i);
		if(peuser)
		{
			delete  peuser;
			peuser = NULL;
		}
	}

	m_aEquimentUser.RemoveAll();
	m_aEquimentUser.SetSize(0);
}


   


int CGmemoryDataManager::LoadAllEquimentInfo()
{
//	return theApp.m_DbQuery.GetEquimentNewestInfo(theApp.m_username,0,this->m_aEuqimentInfo);

	return  1;

}

//查找设备的信息内存;找不到就到数据库里找;
CEquimentInfo * CGmemoryDataManager::FindEquimentInfo(CString szEquimnetId,BOOL  bDB)
{
	if(szEquimnetId.IsEmpty())
	{
		ASSERT(FALSE);
		return  NULL;
	}
	int iCount =this->m_aEuqimentInfo.GetSize();
	for(int i=0;i<iCount;i++)
	{
		CEquimentInfo  *pInfo=(CEquimentInfo *)m_aEuqimentInfo.GetAt(i);
		if(!pInfo)
		{
			ASSERT(FALSE);
			continue;
		}
		if(szEquimnetId==pInfo->EquimentId)
		{
			return  pInfo;
		}
	}
	if(bDB)
	{//数据库查找;
		CEquimentInfo  *p=NULL;
		p=new  CEquimentInfo;
		if(!p)
		{
			return  NULL;
		}
		if(g_DBWork.GetEquimentInfo(szEquimnetId,*p)<0)
		{
			ASSERT(FALSE);
			delete  p;
			p=NULL;
			return  NULL;
		}
		else
		{
			this->m_aEuqimentInfo.Add((CObject  *)p);
			return  p;
		}
	}
	return  NULL;

}

//加载设备的用户信息;
int CGmemoryDataManager::LoadEquimentUser()
{
	CObArray   aEquimentnfo;
	aEquimentnfo.RemoveAll();
	aEquimentnfo.SetSize(0);
	for(int ui=0;ui<m_aEquimentUser.GetSize();ui++)
	{
		CEquimentUser  *peuser=(CEquimentUser *)m_aEquimentUser.GetAt(ui);
		if(peuser)
		{
			delete  peuser;
			peuser=NULL;
		}

	}
	m_aEquimentUser.RemoveAll();
	m_aEquimentUser.SetSize(0);

	if(g_DBWork.GetEquimentInfo(aEquimentnfo)<0)
		return  -1;
	int  iCount=aEquimentnfo.GetSize();
	for(int i=0;i<iCount;i++)
	{
		CEquimentInfo *pEquimentinfo=(CEquimentInfo *)aEquimentnfo.GetAt(i);
		if(!pEquimentinfo)
			continue;
		CString szEquimentId=pEquimentinfo->EquimentId;
		if(szEquimentId.IsEmpty())
			continue;
		//得到相关的设备的user；
        CEquimentUser  *pEuqimentUser=NULL;
		pEuqimentUser=new CEquimentUser(szEquimentId);
		if(!pEuqimentUser)
			continue;
		if(g_DBWork.GetEquimentUser(szEquimentId,pEuqimentUser->m_asUser)<0)
		{
			delete pEuqimentUser;
			pEuqimentUser=NULL;
			continue;
		}
		else
		{//加如内存中;
			this->m_aEquimentUser.Add((CObject *)pEuqimentUser);
		}		
		//end;
	}

	g_DBWork.Free_EquimentInfo(aEquimentnfo);

	return  1;
}

CEquimentUser  * CGmemoryDataManager::GetEquimentUser(const CString &szEquimentId)
{
	 int iCount=this->m_aEquimentUser.GetSize();
	 for(int iLoop=0;iLoop<iCount;iLoop++)
	 {

		 CEquimentUser  *p=NULL;
		 p=(CEquimentUser  *)(m_aEquimentUser.GetAt(iLoop));
		 if(!p)
			 continue;
		 if(p->m_szEquimentId==szEquimentId)
		 {
			 return  p;
		 }
	 }
	 return  NULL;
}

BOOL CGmemoryDataManager::LoadGpsAlarmInfo()
{
	if(g_DBWork.GetGpsAlarmDealWithInfo(m_aGpsAlarmInfo,FALSE)<0)
		return  FALSE;
	return  TRUE;
}



const  CGpsAlarmDealWithInfo * CGmemoryDataManager::FindGpsAlarmInfo(const CString &EuqimentKey, int iState, BOOL bDB)
{
	int  iCount=m_aGpsAlarmInfo.GetSize();
	for(int iLoop=0;iLoop<iCount;iLoop++)
	{
		CGpsAlarmDealWithInfo  *pInof=NULL;
		pInof =(CGpsAlarmDealWithInfo *)m_aGpsAlarmInfo.GetAt(iLoop);
		if(!pInof)
			continue;
		if(EuqimentKey==pInof->EQUIMENTID  && iState ==pInof->STATE )
		{
			return  pInof;
		}       
	}
	if(!bDB)
		return  NULL;
	return  NULL;
	/*
	if(bDB)
	{//在数据库里获取
		CGpsAlarmDealWithInfo  *pNewInfo=NULL;

	}
	*/

}

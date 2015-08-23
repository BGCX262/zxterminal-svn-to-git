// SystemInfo.cpp: implementation of the CSystemInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SystemInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

short  CSystemInfo::m_iPort=5555;  //ϵͳ���������ͻ��˵Ŀ�;
char   CSystemInfo::m_szIP[50]="127.0.0.1";   //�������ip��ַ;
char   CSystemInfo::m_szDBName[50]="NETDB";  //Ҫ���ӵ�db����
char   CSystemInfo::m_szDbPassWsd[50]="GPS2010)&)%Server";//db������;

char   CSystemInfo::m_szUser[50]="sa";

BOOL   CSystemInfo::m_BStart[MAXSERVERTYPE]={FALSE,TRUE,TRUE,TRUE,TRUE,TRUE};




CSystemInfo::CSystemInfo()
{

}

CSystemInfo::~CSystemInfo()
{

}

CString CSystemInfo::GetDBName()
{

	CString  szTemp=m_szDBName;
	return  szTemp;

}

CString CSystemInfo::GetDBPWSD()  
{
	CString  szTemp=m_szDbPassWsd;
	return  szTemp;

}

int   CSystemInfo::GetPort() 
{

	return    m_iPort;
}

CString CSystemInfo::GetDBUser()
{
	CString  szTemp=m_szUser;
	return  szTemp;

}


BOOL CSystemInfo::GetIsServerStart(int  iType)
{
	if(iType<=0  ||  iType>=MAXSERVERTYPE)
		return   FALSE;
	return   m_BStart[iType];
}

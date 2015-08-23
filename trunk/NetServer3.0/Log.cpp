
#include "stdafx.h"
#include "Log.h"


#include "fstream"
using namespace std;

void      g_Log(CString szLog)
{
	static  int icount=0;
	COleDateTime    now=COleDateTime::GetCurrentTime();
	CString  szTemp;
	szTemp.Format("count=%d,  %s     ",icount++,now.Format("%Y-%m-%d %H:%M:%S"));
	szTemp+=szLog;
	CString    szFileName;
	szFileName.Format("Log\\%s.log",now.Format("%Y%m%d"));
	ofstream   f(szFileName.GetBuffer(0),ios::app|ios::out);
	
#ifdef  _DEBUG
	//这个部分负责记录程序在调试的时候的过程;
	char buf[4096]={0};
	sprintf(buf,"%s",szTemp);
    f<<buf<<endl;
	f.close();
	return ;
#endif	
    f<<szTemp.GetBuffer(0)<<endl;
	f.close();
	return ;
	
}
//WorkThread


void      g_EquimentWorkThreadLog(CString szLog)
{
	static  int icount=0;
	COleDateTime    now=COleDateTime::GetCurrentTime();
	CString  szTemp;
	szTemp.Format("count=%d,  %s     ",icount++,now.Format("%Y-%m-%d %H:%M:%S"));
	szTemp+=szLog;
	CString    szFileName;
	szFileName.Format("Log\\EquimentWorkThread%s.log",now.Format("%Y%m%d"));
	ofstream   f(szFileName.GetBuffer(0),ios::app|ios::out);
	
#ifdef  _DEBUG
	//这个部分负责记录程序在调试的时候的过程;
	char buf[1024]={0};
	sprintf(buf,"%s",szTemp);
    f<<buf<<endl;
	f.close();
	return ;
#endif	
    f<<szTemp.GetBuffer(0)<<endl;
	f.close();
	return ;
	
}




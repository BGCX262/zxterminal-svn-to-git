// NetServer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "NetServer.h"
#include "NetServerDlg.h"
#include "DBWork.h"
#include "SystemInfo.h"
#include "GmemoryDataManager.h"
#include <afxmt.h>

#include "controlled_module_ex.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNetServerApp

extern  CDBWork   g_DBWork;


BEGIN_MESSAGE_MAP(CNetServerApp, CWinApp)
	//{{AFX_MSG_MAP(CNetServerApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNetServerApp construction

CNetServerApp::CNetServerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CNetServerApp object

CNetServerApp theApp;
CGmemoryDataManager  g_GmemoryDataManager;
CMutex               g_EquimentUserLock(FALSE,"EquimentUserLock");
CMutex               g_GpsAlarmInfoLock(FALSE,"GpsAlarmInfoLock"); //gps报警信息的lock
CMutex                g_SpeedAlarmLock(FALSE,"SpeedAlarmLock");
CMutex                g_PicMessageLock(FALSE,"PicMessageLock");

class thdex: public controlled_module_ex
{
protected:
	virtual void message(const _command & cmd)
	{
		controlled_module_ex::message(cmd);
		if(cmd.nCmd == BM_USER + 1)
		{
			cout << "get message" << endl;
			CString str = "get message";
			TRACE(str);
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
// CNetServerApp initialization

BOOL CNetServerApp::InitInstance()
{
	WSADATA  wsd;
//	if (!AfxSocketInit())

	
	if(WSAStartup(MAKEWORD(2,2),&wsd)==SOCKET_ERROR)
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
	CSystemInfo  systeminto;
	CString szUser=systeminto.GetDBUser();
	CString szPws=systeminto.GetDBPWSD();
	CString  DataBase=systeminto.GetDBName();
	//if(g_DBWork.ConnectDB(DataBase,szUser,szPws)<0)
	//{
	//	CString szMsg;
	//	szMsg.Format("连接数据库%s失败",DataBase);
	//	AfxMessageBox(szMsg);
	//	return FALSE;
	//}

	//g_SpeedAlarmLock.Lock();
	//m_asSpeedAlarmInfo.RemoveAll();
	//m_asSpeedAlarmInfo.SetSize(0);
	//if(g_DBWork.GetSpeedAlarm(m_asSpeedAlarmInfo)<0)
	//{
	//	AfxMessageBox("加载超速度报警信息失败");
	//	g_SpeedAlarmLock.Unlock();
	//	//	return FALSE;
	//}
	//g_SpeedAlarmLock.Unlock();

	
	//g_EquimentUserLock.Lock();
	//int iRet=g_GmemoryDataManager.LoadEquimentUser();
	//g_EquimentUserLock.Unlock();
	//if(iRet<0)
	//{
	//	AfxMessageBox("加载设备用户列表失败");
	//	return FALSE;
	//}
	//g_GpsAlarmInfoLock.Lock();
	//BOOL   bSucessed=g_GmemoryDataManager.LoadGpsAlarmInfo();
	//g_GpsAlarmInfoLock.Unlock();
	//
	//if(!bSucessed)
	//{
	////	AfxMessageBox("加载监控设备的报警信息失败");
	////	return  TRUE;
	//}
	//m_asPicMessage.RemoveAll();
	//m_asPicMessage.SetSize(0);
	


	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	//thdex t1;
	//t1.safestart();

	//Sleep(5000);

	//thdex t2;
	//t2.safestart();
	//t2.postmessage(BM_USER + 1);
	//char buf[10];
	//gets_s(buf, sizeof buf);
	//t2.safestop();

	try
	{
		if (!bInitializeDB())
		{
			return false;
		}

		CNetServerDlg dlg;
		m_pMainWnd = &dlg;
		int nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with OK
		}
		else if (nResponse == IDCANCEL)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with Cancel
		}
		
	}
	catch (...) {
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	//Sleep(500000000);
	return FALSE;
}

int CNetServerApp::ExitInstance() 
{
	// TODO: Add your specialized code here and/or
	// call the base class.

	g_DBWork.Free_EquimentInfo(theApp.m_asEquimentInfo);

	return CWinApp::ExitInstance();
}

bool CNetServerApp::bInitializeDB()
{
	try
	{
		m_otlDB = new otl_connect();
		m_pDBHelper = new CzxDBHelper(m_otlDB);

		ptree pt;
		read_xml("./NetServer.xml", pt);  

		string strDBHost = pt.get<string>("NetServer.Database.DBHost");
		string strDBName = pt.get<string>("NetServer.Database.DBName");
		string strDBUser = pt.get<string>("NetServer.Database.DBUser");
		string strDBPass = pt.get<string>("NetServer.Database.DBPass");
		//strValue = pt.get<string>("Database.DBPort");

		if (strDBHost == "127.0.0.1" || strDBHost == "localhost")
		{
			strDBHost = "localhost";
		}

		if (!m_pDBHelper->Initialzie(strDBHost, strDBName, strDBUser, strDBPass))
		{
			return false;
		}

		return true;
	}
	catch(otl_exception& p)
	{ 
		// intercept OTL exceptions
		cerr<<p.msg<<endl; // print out error message
		cerr<<p.sqlstate<<endl; // print out SQLSTATE 
		cerr<<p.stm_text<<endl; // print out SQL that caused the error
		cerr<<p.var_info<<endl; // print out the variable that caused the error

		//ACE_ERROR((LM_STARTUP, "%T TH:%t [%M] <%s> Database initialize error [%s] [%s] [%s] [%s]\n", __FUNCTION__,
		//	p.msg, p.sqlstate, p.stm_text, p.var_info));

		return false;
	}

	return true;
}


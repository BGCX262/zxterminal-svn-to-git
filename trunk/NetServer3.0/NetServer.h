// NetServer.h : main header file for the NETSERVER application
//

#if !defined(AFX_NETSERVER_H__C1D49D2F_B32A_40DA_A8A6_46E26AB46648__INCLUDED_)
#define AFX_NETSERVER_H__C1D49D2F_B32A_40DA_A8A6_46E26AB46648__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "ClientManager.h"
#include "GpsFiled.h"

#define OTL_ODBC_MYSQL
#include "otlv4.h"

#include "zxDBHelper.h"

/////////////////////////////////////////////////////////////////////////////
// CNetServerApp:
// See NetServer.cpp for the implementation of this class
//


class CNetServerApp : public CWinApp
{
public:
	TEMP_PicMessage       m_asPicMessage;
	TEMP_SpeedAlarmInfo   m_asSpeedAlarmInfo;
	CNetServerApp();
	CClientManager  m_Clientmanager;
	CObArray        m_asEquimentInfo;
	CMutex          m_AllEquimentInfoLock;

	
	otl_connect* m_otlDB;
	CzxDBHelper* m_pDBHelper;
	bool bInitializeDB();

	//otl_connect GetDBInstance()
	//{
	//	return m_otlDB;
	//}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNetServerApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CNetServerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NETSERVER_H__C1D49D2F_B32A_40DA_A8A6_46E26AB46648__INCLUDED_)

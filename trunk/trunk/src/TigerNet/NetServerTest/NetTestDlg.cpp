
// NetTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NetTest.h"
#include "NetTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNetTestDlg dialog




CNetTestDlg::CNetTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNetTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pTCPServerObj = NULL;

}

void CNetTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RE_RECV_MSG, m_reRecvMsg);
}

BEGIN_MESSAGE_MAP(CNetTestDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_LISTEN, &CNetTestDlg::OnBnClickedBtnListen)
	ON_BN_CLICKED(IDC_BTN_SEND, &CNetTestDlg::OnBnClickedBtnSend)
	ON_EN_SETFOCUS(IDC_EDIT_MSG, &CNetTestDlg::OnEnSetfocusEditMsg)
	ON_EN_KILLFOCUS(IDC_EDIT_MSG, &CNetTestDlg::OnEnKillfocusEditMsg)
	ON_BN_CLICKED(IDC_BUTTON_CONTINUESEND, &CNetTestDlg::OnBnClickedButtonContinuesend)
	ON_BN_CLICKED(IDC_BUTTON_THROUGHPUT, &CNetTestDlg::OnBnClickedButtonThroughput)
	ON_BN_CLICKED(IDOK, &CNetTestDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CNetTestDlg message handlers

BOOL CNetTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	GetDlgItem( IDC_BTN_SEND )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_MSG )->SetWindowText("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

	((CButton*)(GetDlgItem( IDC_RADIO_MDF )))->SetCheck(1);
	m_bMDF = true;

	m_bContinue = false;

	m_ulSended = 0;
	m_ulReceived = 0;
	m_bThroughPut = false;

	GetDlgItem( IDC_EDIT_LENGTH )->SetWindowText( "63" );

	CString strExe;
	GetModuleFileName( NULL, strExe.GetBuffer(MAX_PATH), MAX_PATH );
	strExe.ReleaseBuffer();
	m_strIni = strExe.Left( strExe.ReverseFind('\\') + 1 ) + "MDFServer.ini";

	CString strPort;
	int nPort = GetPrivateProfileInt("RUN","server_port",0,m_strIni);
	if ( nPort>0 )
	{
		strPort.Format("%d",nPort);
		GetDlgItem( IDC_EDIT_SRV_PORT )->SetWindowText( strPort );

		//m_pTCPServerObj = CreateMDFTCPServerInstance( /*nPort,*/this );

		//OnBnClickedBtnListen();
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNetTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNetTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//接受客户端连接回调
void CNetTestDlg::OnAccept( unsigned __int64 unClientID, const char * ClientIP, unsigned short usPort  )
{
	GetDlgItem( IDC_BTN_SEND )->EnableWindow( TRUE );

	m_mapMutex.Lock();
	m_mapClients.insert( mapClient::value_type(unClientID,string(ClientIP)) );

	CString strContent;
	strContent.Format("客户数：%d",m_mapClients.size());
	GetDlgItem( IDC_STATIC_CLIENTNUM )->SetWindowText( strContent );

	m_mapMutex.Unlock();
}

//从服务器接收到数据回调
void CNetTestDlg::OnRecvClientData( unsigned __int64 unClientID, const unsigned char * pucData, unsigned int unDataSize )
{
	char *szData = new char[unDataSize+1];
	memcpy( szData, pucData, unDataSize );
	szData[unDataSize] = '\0';

	m_ulReceived += unDataSize;
	CString strContent;
	strContent.Format("已接收：%lu Byte", m_ulReceived);
	GetDlgItem( IDC_STATIC_RECEIVE )->SetWindowText( strContent );
	strContent.ReleaseBuffer();

	if (m_bThroughPut)
	{
		//m_mapMutex.Lock();
		//mapClient::iterator it = m_mapClients.find( unClientID );
		//while ( it!=m_mapClients.end() )
		{
			if (m_bMDF)
			{
				m_pTCPServerObj->SendDataToMDFClient( unClientID, (unsigned char*)szData, unDataSize );
			} 
			else
			{
				m_pTCPServerObj->SendDataToOtherClient( unClientID, (unsigned char*)szData, unDataSize );
			}

			m_ulSended += unDataSize;
			CString strContent;
			strContent.Format("已发送：%lu Byte",m_ulSended);
			GetDlgItem( IDC_STATIC_SEND )->SetWindowText( strContent );
			strContent.ReleaseBuffer();

			//it ++;
		}

		//m_mapMutex.Unlock();
	} 
	else
	{
		SYSTEMTIME st;
		GetLocalTime( &st );
		CString strTime;
		strTime.Format("%02d:%02d:%02d ",st.wHour,st.wMinute,st.wSecond);

		CString strClientIP="";
		string ClientIP = m_mapClients[ unClientID ];
		if ( !ClientIP.empty() )
		{
			strClientIP.Format("[%s]", ClientIP.c_str() );
		}
		
		m_reRecvMsg.SetSel(-1,-1);
		m_reRecvMsg.ReplaceSel( strClientIP + strTime + CString(szData) );
		m_reRecvMsg.ReplaceSel( "\r\n" ); 
		

		m_reRecvMsg.SendMessage(   WM_VSCROLL,   SB_BOTTOM   ); 

		strTime.ReleaseBuffer();
		strClientIP.ReleaseBuffer();

	}

	delete[] szData;
	szData = NULL;
	
}

//Socket连接断开回调
void CNetTestDlg::OnClientDisconnect(  unsigned __int64 unClientID )
{
	m_mapMutex.Lock();
	mapClient::iterator it = m_mapClients.find( unClientID );
	if ( it!=m_mapClients.end() )
	{
		m_mapClients.erase( it );
	}

	CString strContent;
	strContent.Format("客户数：%d",m_mapClients.size());
	GetDlgItem( IDC_STATIC_CLIENTNUM )->SetWindowText( strContent );

	m_mapMutex.Unlock();

	if (unClientID == 0)
	{
		delete m_pTCPServerObj;
		m_pTCPServerObj = NULL;
		__super::OnCancel();
	}
}

//从服务器接收数据异常回调
void CNetTestDlg::OnClientDataError(unsigned __int64 unClientID)
{

}

void CNetTestDlg::OnBnClickedBtnListen()
{
	// TODO: Add your control notification handler code here
	CString strPort;
	USHORT usPort;
	GetDlgItem( IDC_EDIT_SRV_PORT )->GetWindowText( strPort );
	strPort.TrimLeft();
	strPort.TrimRight();
	if ( strPort.IsEmpty() )
		return;

	usPort = atoi( strPort.GetBuffer(strPort.GetLength()) );
	strPort.ReleaseBuffer();
	if ( usPort<1 )
	{
		return;
	}

	WritePrivateProfileString("RUN","server_port",	strPort, m_strIni );

	if ( m_pTCPServerObj==NULL )
	{
		m_pTCPServerObj = CreateMDFTCPServerInstance( /*usPort,*/this );
	}

	if ( m_pTCPServerObj )
	{
		//m_pTCPServerObj->MDFServerListen( usPort ,false);
		//m_pTCPServerObj->OtherServerListen( usPort ,false);
		//GetDlgItem( IDC_BTN_LISTEN )->EnableWindow( FALSE );

		int nCheck = ((CButton*)(GetDlgItem( IDC_RADIO_MDF )))->GetCheck();
		if (nCheck == 1)
		{
			m_bMDF = true;
			if (m_pTCPServerObj->MDFServerListen( usPort ,false))
			{
				GetDlgItem( IDC_BTN_LISTEN )->EnableWindow( FALSE );
			} 
			else
			{
				AfxMessageBox("端口已被占用");
			}
		}
		else
		{
			m_bMDF = false;
			if (m_pTCPServerObj->OtherServerListen( usPort ,false))
			{
				GetDlgItem( IDC_BTN_LISTEN )->EnableWindow( FALSE );
			} 
			else
			{
				AfxMessageBox("端口已被占用");
			}
		}

		
	}

}

void CNetTestDlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	if ( m_pTCPServerObj )
	{
		m_pTCPServerObj->CloseSocket();

		//m_pTCPServerObj = NULL;

		return;
	}

	__super::OnCancel();
}

void CNetTestDlg::OnBnClickedBtnSend()
{
	// TODO: Add your control notification handler code here

	if ( m_pTCPServerObj==NULL || m_mapClients.size()<1 )
	{
		//GetDlgItem( IDC_EDIT_MSG )->EnableWindow( FALSE );
		return;
	}

	CString strMsg;
	GetDlgItem( IDC_EDIT_MSG )->GetWindowText( strMsg );
	if ( strMsg.IsEmpty() )
	{
		return;
	}
	
	SYSTEMTIME st;
	GetLocalTime( &st );
	CString strTime;
	strTime.Format("%02d:%02d:%02d ",st.wHour,st.wMinute,st.wSecond);
	strMsg = strTime + strMsg;

	//Debug only
	//OnRecvData( (unsigned char*)strMsg.GetBuffer(),strMsg.GetLength() );
	//strMsg.ReleaseBuffer();
	//return;

	//m_pTCPServerObj->SendData( (unsigned char*)strMsg.GetBuffer(strMsg.GetLength()), strMsg.GetLength() );
	//strMsg.ReleaseBuffer();
	m_mapMutex.Lock();
	mapClient::iterator it=m_mapClients.begin();
	while ( it!=m_mapClients.end() )
	{
		if (m_bMDF)
		{
			m_pTCPServerObj->SendDataToMDFClient( it->first, (unsigned char*)strMsg.GetBuffer(strMsg.GetLength()), strMsg.GetLength() );
		} 
		else
		{
			m_pTCPServerObj->SendDataToOtherClient( it->first, (unsigned char*)strMsg.GetBuffer(strMsg.GetLength()), strMsg.GetLength() );
		}

		m_ulSended += strMsg.GetLength();
		CString strContent;
		strContent.Format("已发送：%lu Byte",m_ulSended);
		GetDlgItem( IDC_STATIC_SEND )->SetWindowText( strContent );
		
		it ++;
	}
	strMsg.ReleaseBuffer();
	m_mapMutex.Unlock();
}

void CNetTestDlg::OnEnSetfocusEditMsg()
{
	// TODO: Add your control notification handler code here
	GetDlgItem( IDC_EDIT_MSG )->SetWindowText("");
}

void CNetTestDlg::OnEnKillfocusEditMsg()
{
	// TODO: Add your control notification handler code here
	CString strText;
	GetDlgItem( IDC_EDIT_MSG )->GetWindowText(strText);
	if ( strText.IsEmpty() )
	{
		GetDlgItem( IDC_EDIT_MSG )->SetWindowText("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	}
}

void CNetTestDlg::OnBnClickedButtonContinuesend()
{
	// TODO: 在此添加控件通知处理程序代码

	m_ulSended = 0;

	if (!m_bContinue)
	{
		m_bContinue = true;
	}
	else
	{
		m_bContinue = false;
		return;
	}

	if ( m_pTCPServerObj==NULL || m_mapClients.size()<1 )
	{
		//GetDlgItem( IDC_EDIT_MSG )->EnableWindow( FALSE );
		return;
	}

	//DWORD dwThreadID = 0;
	//HANDLE hSendThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)&(CNetTestDlg::ContinueSend),NULL,0,&dwThreadID);


	CString strLength;
	GetDlgItem( IDC_EDIT_LENGTH )->GetWindowText( strLength );
	int nLength = atoi(strLength.GetBuffer(strLength.GetLength()));
	strLength.ReleaseBuffer();

	nLength = nLength * 1024;
	char* szMsg = new char[nLength + 1];
	memset(szMsg,'A',nLength);
	szMsg[nLength] = '\0';

	strLength.ReleaseBuffer();

	CString strMsg(szMsg);
	
	while (m_bContinue)
	{
		m_mapMutex.Lock();
		mapClient::iterator it=m_mapClients.begin();
		while ( it!=m_mapClients.end() )
		{
			if (m_bMDF)
			{
				m_pTCPServerObj->SendDataToMDFClient( it->first, (unsigned char*)strMsg.GetBuffer(strMsg.GetLength()), strMsg.GetLength() );
			} 
			else
			{
				m_pTCPServerObj->SendDataToOtherClient( it->first, (unsigned char*)strMsg.GetBuffer(strMsg.GetLength()), strMsg.GetLength() );
			}

			m_ulSended += strMsg.GetLength();
			CString strContent;
			strContent.Format("已发送：%lu Byte",m_ulSended);
			GetDlgItem( IDC_STATIC_SEND )->SetWindowText( strContent );

			it ++;
		}
		strMsg.ReleaseBuffer();
		m_mapMutex.Unlock();

		Sleep(1000);
	}

}


void CNetTestDlg::ContinueSend()
//void ContinueSend()
{
	CString strMsg;
	GetDlgItem( IDC_EDIT_MSG )->GetWindowText( strMsg );
	if ( strMsg.IsEmpty() )
	{
		return;
	}

	SYSTEMTIME st;
	GetLocalTime( &st );
	CString strTime;
	strTime.Format("%02d:%02d:%02d ",st.wHour,st.wMinute,st.wSecond);
	strMsg = strTime + strMsg;

	while (m_bContinue)
	{
		m_mapMutex.Lock();
		mapClient::iterator it=m_mapClients.begin();
		while ( it!=m_mapClients.end() )
		{
			if (m_bMDF)
			{
				m_pTCPServerObj->SendDataToMDFClient( it->first, (unsigned char*)strMsg.GetBuffer(strMsg.GetLength()), strMsg.GetLength() );
			} 
			else
			{
				m_pTCPServerObj->SendDataToOtherClient( it->first, (unsigned char*)strMsg.GetBuffer(strMsg.GetLength()), strMsg.GetLength() );
			}

			m_ulSended += strMsg.GetLength();
			CString strContent;
			strContent.Format("已发送：%lu Byte",m_ulSended);
			GetDlgItem( IDC_STATIC_SEND )->SetWindowText( strContent );

			it ++;
		}
		Sleep(5000);
		strMsg.ReleaseBuffer();
		m_mapMutex.Unlock();
	}

}

void CNetTestDlg::OnBnClickedButtonThroughput()
{
	// TODO: 婓森氝樓諷璃籵眭揭燴最唗測鎢
	if (m_bThroughPut)
	{
		m_bThroughPut = false;

		long lNow = (long)time(0);

		if (lNow - m_lThroughPutBegin == 0)
		{
			AfxMessageBox(_T("时间间隔太短，无法计算"));
		}
		else
		{
			INT64 ulThrough = (m_ulSended + m_ulReceived) / (lNow - m_lThroughPutBegin) / 1024;

			CString strContent;
			strContent.Format("吞吐量：%lu KByte/s",ulThrough);
			GetDlgItem( IDC_STATIC_THROUGHPUT )->SetWindowText( strContent );
			strContent.ReleaseBuffer();
		}
	} 
	else
	{
		m_bThroughPut = true;

		m_lThroughPutBegin = (long)time(0);

		m_ulSended = 0;
		m_ulReceived = 0;


		CString strLength;
		GetDlgItem( IDC_EDIT_LENGTH )->GetWindowText( strLength );
		int nLength = atoi(strLength.GetBuffer(strLength.GetLength()));
		strLength.ReleaseBuffer();

		nLength = nLength * 1024;
		char* szMsg = new char[nLength + 1];
		memset(szMsg,'A',nLength);
		szMsg[nLength] = '\0';

		strLength.ReleaseBuffer();
		
		CString strMsg(szMsg);
		//GetDlgItem( IDC_EDIT_MSG )->GetWindowText( strMsg );
		//if ( strMsg.IsEmpty() )
		//{
		//	return;
		//}

		m_mapMutex.Lock();
		mapClient::iterator it=m_mapClients.begin();
		while ( it!=m_mapClients.end() )
		{
			if (m_bMDF)
			{
				//m_pTCPServerObj->SendDataToMDFClient( it->first, (unsigned char*)strMsg.GetBuffer(strMsg.GetLength()), strMsg.GetLength() );
				m_pTCPServerObj->SendDataToMDFClient( it->first, (unsigned char*)szMsg, nLength );
			} 
			else
			{
				m_pTCPServerObj->SendDataToOtherClient( it->first, (unsigned char*)strMsg.GetBuffer(strMsg.GetLength()), strMsg.GetLength() );
			}

			//m_ulSended += strMsg.GetLength();
			m_ulSended += nLength;
			CString strContent;
			strContent.Format("已发送：%lu Byte",m_ulSended);
			GetDlgItem( IDC_STATIC_SEND )->SetWindowText( strContent );
			strContent.ReleaseBuffer();

			it ++;
		}

		m_mapMutex.Unlock();

		delete[] szMsg;
		szMsg = NULL;
	}
}

void CNetTestDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	OnCancel();
}

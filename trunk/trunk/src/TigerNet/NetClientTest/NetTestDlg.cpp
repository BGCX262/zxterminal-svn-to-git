
// NetTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NetTest.h"
#include "NetTestDlg.h"

//#include <boost/bind.hpp>
//#include <boost/asio.hpp>
#include <boost/thread.hpp>

using namespace boost;

//#include "../../Include/vld.h"
//#include "../../Include/vldapi.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNetTestDlg dialog




CNetTestDlg::CNetTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNetTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pTCPClientObj = NULL;

	m_bExit		= false;
	m_bNotifyExit = false;
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
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CNetTestDlg::OnBnClickedBtnConnect)
	ON_BN_CLICKED(IDC_BTN_SEND, &CNetTestDlg::OnBnClickedBtnSend)
	ON_EN_SETFOCUS(IDC_EDIT_MSG, &CNetTestDlg::OnEnSetfocusEditMsg)
	ON_EN_KILLFOCUS(IDC_EDIT_MSG, &CNetTestDlg::OnEnKillfocusEditMsg)
	ON_BN_CLICKED(IDC_BUTTON_THROUGHPUT, &CNetTestDlg::OnBnClickedButtonThroughput)
	ON_BN_CLICKED(IDC_BUTTON_CONCURRENCY, &CNetTestDlg::OnBnClickedButtonConcurrency)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CNetTestDlg::OnBnClickedButtonDisconnect)
END_MESSAGE_MAP()


// CNetTestDlg message handlers

BOOL CNetTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	GetDlgItem( IDC_BTN_SEND )->EnableWindow( FALSE );
	GetDlgItem( IDC_EDIT_MSG )->SetWindowText("1234567890");

	m_bThroughPut = false;
	m_ulSended = 0;
	m_ulReceived = 0;

	CString strExe;
	GetModuleFileName( NULL, strExe.GetBuffer(MAX_PATH), MAX_PATH );
	strExe.ReleaseBuffer();
	m_strIni = strExe.Left( strExe.ReverseFind('\\') + 1 ) + "MDFClient.ini";

	CString strIP;
	GetPrivateProfileString("RUN","server_ip","127.0.0.1",strIP.GetBuffer(64),64,m_strIni);
	strIP.TrimLeft();
	strIP.TrimRight();
	GetDlgItem( IDC_EDIT_SRV_IP )->SetWindowText( strIP );

	CString strPort;
	int nPort = GetPrivateProfileInt("RUN","server_port",0,m_strIni);
	if ( nPort>0 )
	{
		strPort.Format("%d",nPort);
		GetDlgItem( IDC_EDIT_SRV_PORT )->SetWindowText( strPort );
	}

	if ( !strPort.IsEmpty() && nPort>0 )
	{
		OnBnClickedBtnConnect();
		OnBnClickedButtonThroughput();
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

//连接服务器回调函数，由nStatus指定连接是否成功。
void CNetTestDlg::OnConnectServer( int nStatus )
{
	TRACE("Connect status=%d\n", nStatus );

	switch ( nStatus )
	{
	case -1://网络断开处理
		GetDlgItem( IDC_BTN_SEND )->EnableWindow( FALSE );
		GetDlgItem( IDC_BTN_CONNECT )->EnableWindow( TRUE );
		//重连
		//OnBnClickedBtnConnect();

		break;

	case 0:
		GetDlgItem( IDC_BTN_SEND )->EnableWindow( TRUE );
		break;

	case -15:
		GetDlgItem( IDC_BTN_CONNECT )->EnableWindow( TRUE );
		break;

	default:
		break;
	}

}

//从服务器接收到数据回调
void CNetTestDlg::OnRecvServerData( const unsigned char * pucData, unsigned int unDataSize )
{
	m_ulReceived += unDataSize;
	CString strContent;
	strContent.Format("已接收：%lu Byte", m_ulReceived);
	GetDlgItem( IDC_STATIC_RECEIVE )->SetWindowText( strContent );

	//char *szData = new char[unDataSize +  1];
	//memcpy( szData, pucData, unDataSize );
	//szData[unDataSize] = '\0';

	if (m_bThroughPut)
	{
		if (m_pTCPClientObj)
		{
			//TRACE("SendDataToMDFServer\n" );
			m_pTCPClientObj->SendDataToOtherServer( (unsigned char*)pucData, unDataSize );
		}
		

		m_ulSended += unDataSize;
		CString strContent;
		strContent.Format("已发送：%lu Byte", m_ulSended);
		//TRACE("OnRecvServerData set begin\n" );
		GetDlgItem( IDC_STATIC_SEND )->SetWindowText( strContent );
		//TRACE("OnRecvServerData set end\n" );

		SYSTEMTIME st;
		GetLocalTime( &st );
		CString strTime;
		strTime.Format("%02d:%02d:%02d ",st.wHour,st.wMinute,st.wSecond);

		
		//m_reRecvMsg.SetSel(-1,-1);
		//m_reRecvMsg.ReplaceSel( strTime + CString(szData) );
		//m_reRecvMsg.ReplaceSel( "\r\n" ); 

		//m_reRecvMsg.SendMessage(   WM_VSCROLL,   SB_BOTTOM   ); 
	} 
	////else
	//{
	//	SYSTEMTIME st;
	//	GetLocalTime( &st );
	//	CString strTime;
	//	strTime.Format("%02d:%02d:%02d ",st.wHour,st.wMinute,st.wSecond);

	//	
	//	m_reRecvMsg.SetSel(-1,-1);
	//	m_reRecvMsg.ReplaceSel( strTime + CString(szData) );
	//	m_reRecvMsg.ReplaceSel( "\r\n" ); 
	//	delete[] szData;
	//	szData = NULL;

	//	m_reRecvMsg.SendMessage(   WM_VSCROLL,   SB_BOTTOM   ); 
	//}

	//delete[] szData;
	//szData = NULL;

	//Sleep(10);
	//TRACE("OnRecvServerData  end\n" );
	

}

//Socket连接断开回调
void CNetTestDlg::OnServerDisconnect( int nStatus )
{
	if (m_bNotifyExit)
	{
		return;
	}

	SYSTEMTIME st;
	GetLocalTime(&st);
	
	TRACE("%02d:%02d:%02d OnServerDisconnect\n", st.wHour,st.wMinute,st.wSecond );
	//if (nStatus == -14)
	//{
	//	//AfxMessageBox("超过45秒没有收到数据");
	//} 

	//

	//if (nStatus != -15)
	//{
	//	GetDlgItem( IDC_BTN_SEND )->EnableWindow( FALSE );
	//	GetDlgItem( IDC_BTN_CONNECT )->EnableWindow( TRUE );
	//} 
	//
	//if (m_pTCPClientObj)
	//{
	//	delete m_pTCPClientObj;
	//	m_pTCPClientObj = NULL;
	//}

	GetDlgItem( IDC_BTN_SEND )->EnableWindow( FALSE );
	GetDlgItem( IDC_BTN_CONNECT )->EnableWindow( TRUE );

	//if ( nStatus == -15 )
	//{
	//	OnBnClickedBtnConnect();

	//}

	if ( nStatus == -16 )	//SDK通知资源释放完毕
	{
		if ( m_bExit )	//发出退出APP请求
		{
			if ( m_pTCPClientObj )
			{
				delete m_pTCPClientObj;
				m_pTCPClientObj = NULL;
			}

			m_bNotifyExit = true;
			__super::OnCancel();
			
			return;		
		}

	}
}

//从服务器接收数据异常回调
void CNetTestDlg::OnServerDataError()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	TRACE("%02d:%02d:%02d OnServerDataError\n", st.wHour,st.wMinute,st.wSecond );

}

void CNetTestDlg::OnBnClickedBtnConnect()
{
	// TODO: Add your control notification handler code here
	CString strIP,strPort;
	USHORT usPort;
	GetDlgItem( IDC_EDIT_SRV_IP )->GetWindowText( strIP );
	GetDlgItem( IDC_EDIT_SRV_PORT )->GetWindowText( strPort );

	strIP.TrimLeft();
	strIP.TrimRight();
	if ( strIP.IsEmpty() )
		return;

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

	//Save configuration parameters
	WritePrivateProfileString("RUN","server_ip",	strIP,	 m_strIni);
	WritePrivateProfileString("RUN","server_port",	strPort, m_strIni);

	if ( m_pTCPClientObj==NULL )
	{
		m_pTCPClientObj = CreateMDFTCPClientInstance( this );
		if ( m_pTCPClientObj )
		{
			//m_pTCPClientObj->ConnectMDFServer( strIP.GetBuffer(strIP.GetLength()), usPort ,false);
			m_pTCPClientObj->ConnectOtherServer( strIP.GetBuffer(strIP.GetLength()), usPort ,false);
			strPort.ReleaseBuffer();

			GetDlgItem( IDC_BTN_CONNECT )->EnableWindow( FALSE );
		}	
	}
	else
	{
		m_pTCPClientObj->ConnectOtherServer( strIP.GetBuffer(strIP.GetLength()), usPort ,false);
		strPort.ReleaseBuffer();

		GetDlgItem( IDC_BTN_CONNECT )->EnableWindow( FALSE );

	}
}

void CNetTestDlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	m_bExit = true;

	if ( m_pTCPClientObj )
	{
		TRACE("begin CloseSocket\n");
		m_pTCPClientObj->CloseSocket(1);
		TRACE("end CloseSocket\n");

		//Sleep(5000);
		//if (m_pTCPClientObj)
		//{
		//	delete m_pTCPClientObj;
		//	m_pTCPClientObj = NULL;
		//}

		//__super::OnCancel();

		return;

		
	}

	//delete m_pTCPClientObj;
	//m_pTCPClientObj = NULL;

	TRACE("begin OnCancel\n");
	__super::OnCancel();
	TRACE("end OnCancel\n");
}

void CNetTestDlg::OnBnClickedBtnSend()
{
	// TODO: Add your control notification handler code here

	if ( m_pTCPClientObj==NULL )
	{
		GetDlgItem( IDC_EDIT_MSG )->EnableWindow( FALSE );
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
	//OnRecvData( (BYTE*)strMsg.GetBuffer(),strMsg.GetLength() );
	//strMsg.ReleaseBuffer();
	//return;

	m_pTCPClientObj->SendDataToMDFServer( (unsigned char*)strMsg.GetBuffer(strMsg.GetLength()), strMsg.GetLength() );
	strMsg.ReleaseBuffer();
	strTime.ReleaseBuffer();

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
		GetDlgItem( IDC_EDIT_MSG )->SetWindowText("1234567890");
	}
}

void CNetTestDlg::OnBnClickedButtonThroughput()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bThroughPut)
	{
		m_bThroughPut = false;
	} 
	else
	{
		m_bThroughPut = true;
		m_ulSended = 0;
		m_ulReceived = 0;
	}
}

void CNetTestDlg::OnBnClickedButtonConcurrency()
{
	// TODO: 在此添加控件通知处理程序代码
	

	thread ConnThread(&CNetTestDlg::ConnectThread,this);//,strPort,usPort,nNum

}

void CNetTestDlg::ConnectThread()//CString strIP,int nPort,int nNum
{
	CString strIP,strPort;
	USHORT usPort;
	GetDlgItem( IDC_EDIT_SRV_IP )->GetWindowText( strIP );
	GetDlgItem( IDC_EDIT_SRV_PORT )->GetWindowText( strPort );

	strIP.TrimLeft();
	strIP.TrimRight();
	if ( strIP.IsEmpty() )
		return;

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


	CString strText;
	GetDlgItem(IDC_EDIT_CLIENTNUM)->GetWindowText(strText);
	int nNum = atoi(strText.GetBuffer(strText.GetLength()));

	for (int i = 0; i < nNum; i ++)
	{
		ITCPClient* pClient = CreateMDFTCPClientInstance( this );
		pClient->ConnectMDFServer( strIP.GetBuffer(strIP.GetLength()), usPort ,false);
		Sleep(1000);
	}

}

void CNetTestDlg::OnBnClickedButtonDisconnect()
{
	// TODO: 在此添加控件通知处理程序代码

	if ( m_pTCPClientObj )
	{
		TRACE("begin CloseSocket\n");
		m_pTCPClientObj->CloseSocket(0);
		TRACE("end CloseSocket\n");

		GetDlgItem( IDC_BTN_CONNECT )->EnableWindow( TRUE );
		//GetDlgItem( IDC_BUTTON_DISCONNECT )->EnableWindow( FALSE );

	}
}

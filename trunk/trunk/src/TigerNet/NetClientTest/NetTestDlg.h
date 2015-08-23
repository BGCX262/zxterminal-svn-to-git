
// NetTestDlg.h : header file
//

#pragma once
#include "afxcmn.h"

// CNetTestDlg dialog
class CNetTestDlg : public CDialog,public ITCPClientNotify
{
// Construction
public:
	CNetTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_NETTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	//连接服务器回调函数，由nStatus指定连接是否成功。
	virtual void OnConnectServer( int nStatus );

	//从服务器接收到数据回调
	virtual void OnRecvServerData( const unsigned char * pucData, unsigned int unDataSize );

	//从服务器接收数据异常回调
	virtual void OnServerDataError();

	//Socket连接断开回调
	virtual void OnServerDisconnect( int nStatus );

// Implementation
protected:
	HICON m_hIcon;
	ITCPClient *m_pTCPClientObj;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnConnect();
protected:
	virtual void OnCancel();
public:
	afx_msg void OnBnClickedBtnSend();
	afx_msg void OnEnSetfocusEditMsg();
	afx_msg void OnEnKillfocusEditMsg();
private:
	CRichEditCtrl m_reRecvMsg;
	CString m_strIni;

	bool m_bThroughPut;

	INT64 m_ulSended;
	INT64 m_ulReceived;

public:
	afx_msg void OnBnClickedButtonThroughput();
	afx_msg void OnBnClickedButtonConcurrency();

	void ConnectThread();//CString strIP,int nPort,int nNum

	bool m_bExit;	//退出APP标志
	bool m_bNotifyExit;
	afx_msg void OnBnClickedButtonDisconnect();
};

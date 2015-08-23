
// NetTestDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxmt.h"

#include <map>

using namespace std;
typedef map<unsigned __int64,string> mapClient;

// CNetTestDlg dialog
class CNetTestDlg : public CDialog,public ITCPServerNotify
{
// Construction
public:
	CNetTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_NETTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
	//接受客户端连接回调
	virtual void OnAccept( unsigned __int64 unClientID, const char * ClientIP , unsigned short usPort );

	//从客户端接收到数据回调
	virtual void OnRecvClientData( unsigned __int64 unClientID, const unsigned char * pucData, unsigned int unDataSize );

	//从客户端接收数据异常回调
	virtual void OnClientDataError(unsigned __int64 unClientID);

	//断开客户端网络连接回调
	virtual void OnClientDisconnect( unsigned __int64 unClientID );

	void ContinueSend();

// Implementation
protected:
	HICON m_hIcon;
	ITCPServer *m_pTCPServerObj;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnListen();
protected:
	virtual void OnCancel();
public:
	afx_msg void OnBnClickedBtnSend();
	afx_msg void OnEnSetfocusEditMsg();
	afx_msg void OnEnKillfocusEditMsg();
private:
	CRichEditCtrl m_reRecvMsg;
	mapClient m_mapClients;
	CMutex m_mapMutex;
	CString m_strIni;

	bool m_bMDF;
	bool m_bContinue;
	INT64 m_ulSended;
	INT64 m_ulReceived;
	bool m_bThroughPut;
	long m_lThroughPutBegin;
public:
	afx_msg void OnBnClickedButtonContinuesend();
	afx_msg void OnBnClickedButtonThroughput();
	afx_msg void OnBnClickedOk();
};

// NetServerDlg.h : header file
//

#if !defined(AFX_NETSERVERDLG_H__2A2C65F6_95AC_42C6_B695_544AAD673F7E__INCLUDED_)
#define AFX_NETSERVERDLG_H__2A2C65F6_95AC_42C6_B695_544AAD673F7E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CNetServerDlg dialog

#include "SocketService.h"
#include "UDPSocketService.h"


#include "CommField.h"
#include "Mapx.h"
#include "DBWork.h"

#include <map>
#include <vector>
using namespace std;

#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/xml_parser.hpp>  
#include <boost/typeof/typeof.hpp>   

#include <boost/algorithm/string.hpp>  
#include <boost/foreach.hpp>  

using boost::property_tree::ptree;



#define    DYNAMICLAYER  "tempdrawlayer"

class  SlowDownInfo 
{
public:
	char   szBeginTime[20];   //开始时间;
	int    nExceptionNum;     //异常次数

	SlowDownInfo()
	{
		memset(szBeginTime, 0, 20);
		nExceptionNum = 0;
	}
};

class CNetServerDlg : public CDialog
{

// Construction
public:
	CString   m_PreDianchanRun;

	COleDateTime   m_oleSendDianchanRun;

	BOOL StartDbThread(void *p);
	void WriteGpsDateToDb(CGPSData  *pDate);
	void DealWithEquimentSetTime();
	void DoCheckHeatBeat();		
	void UpdateDbAllInfo();

	CNetServerDlg(CWnd* pParent = NULL);	// standard constructor
	CSocketService  m_SocketServer;

	CMapX  m_Map;
	CString GetCurrentAllPath();
	int IsPointInLayer(CMapX &Map,double x, double y, CString szLayerName,BOOL  &ISIN);
	
	map<CString, CAreaInfo> m_mapAreaInfo;
	map<CString, CMapXLayer> m_mapLayer;
	CDBWork   m_DBWork;

	map<CString, CGPSData*> m_mapGPSData;
	map<CString, SlowDownInfo> m_mapExceptionInfo;
	CMapXLayer  CreateLayer(CMapX &Map, CString szLayerName); //创建一个临时图层
	void CheckBlindArea(CString strEquipmentID, CString strLayerName);
	void CheckSlowDown(CGPSData* pPreData, CGPSData*pGpsData);
	BOOL IsMineCar(CGPSData* pData);

	void CheckDeviceStatus();
	bool NotifyDeviceStatusChange(std::string strDeviceID, int nIsRun, int nHaltReason);



// Dialog Data
	//{{AFX_DATA(CNetServerDlg)
	enum { IDD = IDD_NETSERVER_DIALOG };
	CButton	m_btnExit;
	CListCtrl	m_MineCarList;
	CListCtrl	m_EdipperList;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNetServerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CNetServerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	
	afx_msg void OnClose();
	afx_msg LRESULT OnConnect(WPARAM wparam, LPARAM lParam);
	afx_msg LRESULT OnDisConnect(WPARAM wparam, LPARAM lParam);
	virtual void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg LRESULT OnDoPIC(WPARAM wparam,LPARAM lParam);
	afx_msg LRESULT OnGPSDATA(WPARAM wparam,LPARAM lParam);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void Start();
	bool InitData();
	bool LoadEDipperData();
	bool LoadMineCarData();
	bool LoadAreaData();

	bool SendCommonMessage(int fd, CommonMessage commMessage);
public:
	afx_msg void OnBnClickedExit();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NETSERVERDLG_H__2A2C65F6_95AC_42C6_B695_544AAD673F7E__INCLUDED_)






















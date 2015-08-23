// NetServerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NetServer.h"
#include "NetServerDlg.h"
#include "DBWork.h"
#include "GuardFiled.h"
#include "GmemoryDataManager.h"
#include "GpsFiled.h"


#include "SystemInfo.h"
//#include "SocketService.h"
//#include "NetField.h"
#include <fstream>
using namespace std;

#include "MapXReplace.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/condition_variable.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CMutex      m_NetNodeLock; 

HWND g_NetServerDlgHandle;
std::map<string, std::vector<string>> g_mapRequestTable;


std::map<string, zxClient> g_mapClients;
boost::mutex g_mapClients_mutex;

std::map<std::string, zxTaskRecord> g_mapTaskRecords;
std::map<int, CzxArea> g_mapAreas;

std::map<std::string, GPSData> g_mapGPSData;
boost::mutex g_mapGPSData_mutex;

std::map<string, zxEdipper> g_mapEdippers;
boost::mutex g_mapEdippers_mutex;

std::map<std::string, zxMineCar> g_mapMineCars;
boost::mutex g_mapMineCars_mutex;



/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
/*
*/

extern CGmemoryDataManager  g_GmemoryDataManager;
extern CMutex               g_EquimentUserLock;
extern CMutex               g_GpsAlarmInfoLock; //gps报警信息的lock
extern CMutex  g_SpeedAlarmLock;
extern CMutex  g_PicMessageLock;

#define     DEF_DB_WEB_TIMER    5
#define     DEF_DB_WEB_TIME     1000*30    //web

#define     DEF_DB_SMS_TIMER     6
#define     DEF_DB_SMS_TIME      1000   //SMS

//检测服务器ip地址
#define     DEF_CHECKSERVERIP_TIMER  9  
#define     DEF_CHECKSERVERIP_TIME   1000 *6  //6秒检测一次;

//更新所有的数据库里的设备和用户的信息;
#define     DEF_UPDATEDBALLINFO_TIMER  10 
#define     DEF_UPDATEDBALLINFO_TIME   1000*40   //40秒检测一次;

//心跳检测
#define    DEF_HEARTBEAT_TIMER        11
#define    DEF_HEARTBEAT_TIME        1000*5     //5秒钟检测一次;

//设备状态检测
#define    DEF_DEVICE_CHECK_TIMER        12
#define    DEF_DEVICE_CHECK_TIME        1000*30     //一分钟检测一次;


#define  WM_PICMESSAGE   WM_USER + 70 

CDBWork   g_DBWork;


extern CNetServerApp theApp;
extern CMutex  g_ClientManagerLock;

CMutex     g_GpsDateArrayLock(FALSE,"GpsDateArrayLock");  //数据存储的锁
CObArray   g_GpsDateArray;  //用来接受gps数据的缓冲区

void  AreaLog(char* buf)
{
	ofstream  f("AreaLog.Log", ios::app | ios::out);
	f << buf << endl;
	f.close();
}

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNetServerDlg dialog

//写数据库的进程;
UINT GPSDBDateWriteThread(LPVOID  pParmer)
{

	CNetServerDlg  *pThis=(CNetServerDlg *)pParmer;
	if(!pThis)
	{
		AfxMessageBox("数据存储模块位启动");
		return  0;
		
	}
	while (TRUE)
	{
		
		g_GpsDateArrayLock.Lock();
		int  iCount=g_GpsDateArray.GetSize();
		 if(iCount>0)
		 {
			 CGPSData  *pDate=(CGPSData *)g_GpsDateArray.GetAt(0);
			 if(pDate)
			 {
				 pThis->WriteGpsDateToDb(pDate);

			 }
			 delete  pDate;
			 pDate=NULL;
			 g_GpsDateArray.RemoveAt(0);
		 
		 }
		 else
		 {
			 g_GpsDateArrayLock.Unlock();
			 Sleep(1000);
		 }
		g_GpsDateArrayLock.Unlock();
		
	}

	return  0;
}

CNetServerDlg::CNetServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNetServerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNetServerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNetServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNetServerDlg)
	DDX_Control(pDX, IDC_EXIT, m_btnExit);
	DDX_Control(pDX, IDC_ONLINE_USERLIST, m_MineCarList);
	DDX_Control(pDX, IDC_EQU_ONLINE_LIST, m_EdipperList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNetServerDlg, CDialog)
	//{{AFX_MSG_MAP(CNetServerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_CONNECT, OnConnect)
	ON_MESSAGE(WM_DISCONNECT, OnDisConnect)
	ON_WM_TIMER()
	ON_MESSAGE(WM_PICMESSAGE, OnDoPIC)
	ON_MESSAGE(WM_GPSDATA_MSG, OnGPSDATA)
	ON_BN_CLICKED(IDC_EXIT, &CNetServerDlg::OnBnClickedExit)
	//}}AFX_MSG_MAP
	
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNetServerDlg message handlers

CString CNetServerDlg::GetCurrentAllPath()
{
	TCHAR szModule[_MAX_PATH];
	//Current Directory
	::GetCurrentDirectory(_MAX_PATH, szModule);
	CString  szPath=szModule;
	return  szPath;
}

BOOL CNetServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	
	// TODO: Add extra initialization here
	//用户列表;
	DWORD styleFlag = this->m_MineCarList.GetExtendedStyle();
	styleFlag = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES;
	this->m_MineCarList.SetExtendedStyle(styleFlag);
	this->m_MineCarList.InsertColumn(0, "用户ID", LVCFMT_LEFT, 100);
	this->m_MineCarList.InsertColumn(2, "登陆时间", LVCFMT_LEFT, 150);
	this->m_MineCarList.InsertColumn(1, "登陆地址", LVCFMT_LEFT, 150);
	//设备列表
	styleFlag = this->m_EdipperList.GetExtendedStyle();
	styleFlag = LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES;
	this->m_EdipperList.SetExtendedStyle(styleFlag);
	this->m_EdipperList.InsertColumn(0, "设备标识", LVCFMT_LEFT, 100);
	this->m_EdipperList.InsertColumn(1, "登陆时间", LVCFMT_LEFT, 150);
	this->m_EdipperList.InsertColumn(2, "登陆IP", LVCFMT_LEFT, 150);

	InitData();

	Start();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

bool CNetServerDlg::LoadEDipperData()
{
	return theApp.m_pDBHelper->GetEDippers(g_mapEdippers);
}

bool CNetServerDlg::LoadMineCarData()
{
	return theApp.m_pDBHelper->GetMineCars(g_mapMineCars);
}

bool CNetServerDlg::LoadAreaData()
{
	return theApp.m_pDBHelper->GetAreas(g_mapAreas);
}

bool CNetServerDlg::InitData()
{
	LoadEDipperData();
	LoadMineCarData();
	LoadAreaData();

	return true;
}

void CNetServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNetServerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNetServerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CNetServerDlg::Start() 
{
	BOOL bNetOk = m_SocketServer.StartService();
	if(bNetOk)
	{
		this->SetWindowText("GPS网络服务器---启动成功");
		this->SetTimer(DEF_DB_WEB_TIMER,DEF_DB_WEB_TIME,NULL);
		this->SetTimer(DEF_DB_SMS_TIMER,DEF_DB_SMS_TIME,NULL);
		this->SetTimer(DEF_CHECKSERVERIP_TIMER,DEF_CHECKSERVERIP_TIME,NULL);
		this->SetTimer(DEF_UPDATEDBALLINFO_TIMER,DEF_UPDATEDBALLINFO_TIME,NULL);
		this->SetTimer(DEF_HEARTBEAT_TIMER,DEF_HEARTBEAT_TIME,NULL);

		this->SetTimer(DEF_DEVICE_CHECK_TIMER, DEF_DEVICE_CHECK_TIME, NULL);
	}
	else
	{
		this->SetWindowText("GPS网络服务器---启动失败");
	}
}

void CNetServerDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	OnBnClickedExit();
}

void CNetServerDlg::OnCancel() 
{
	if(MessageBox( TEXT("是否确认退出系统"), TEXT("警告"), MB_OKCANCEL)==IDOK)
	{	
		m_SocketServer.EndService();
		
		g_DBWork.DisConnectDB();

		CDialog::OnCancel();
	}
}

LRESULT CNetServerDlg::OnConnect(WPARAM wparam, LPARAM lParam)
{
	zxClient* client = (zxClient*)wparam;
	if(lParam == 1)
	{
		if (client->nType == MINE_CAR)
		{
			int iCount = m_MineCarList.GetItemCount();
			this->m_MineCarList.InsertItem(iCount, client->szDeviceID);
			this->m_MineCarList.SetItemText(iCount, 1, client->szDeviceName);
			this->m_MineCarList.SetItemText(iCount, 2, client->szLoginTime);
		} 
		else if (client->nType == E_DIPPER)
		{
			int iCount = m_EdipperList.GetItemCount();
			this->m_EdipperList.InsertItem(iCount, client->szDeviceID);
			this->m_EdipperList.SetItemText(iCount, 1, client->szDeviceName);
			this->m_EdipperList.SetItemText(iCount, 2, client->szLoginTime);
		}
	}

	return S_OK;
}

LRESULT CNetServerDlg::OnDisConnect(WPARAM wparam, LPARAM lParam)
{
	zxClient* client = (zxClient*)wparam;
	if(lParam == 1)
	{
		if (client->nType == MINE_CAR)
		{
			int iCount = m_MineCarList.GetItemCount();
			for(int i = 0; i < iCount; i ++)
			{
				CString strDeviceID = client->szDeviceID;
				CString szTemp = m_MineCarList.GetItemText(i, 0);
				if(strDeviceID == szTemp)
				{
					m_MineCarList.DeleteItem(i);
					break;
				}
			}
		} 
		else if (client->nType == E_DIPPER)
		{
			int iCount = m_EdipperList.GetItemCount();
			for(int i = 0; i < iCount; i ++)
			{
				CString strDeviceID = client->szDeviceID;
				CString szTemp = m_EdipperList.GetItemText(i, 0);
				if(strDeviceID == szTemp)
				{
					m_EdipperList.DeleteItem(i);
					break;
				}
			}
		}
	}

	return S_OK;
}

void CNetServerDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent==DEF_HEARTBEAT_TIMER)
	{
		//心跳检测
		DoCheckHeatBeat();
	}

	if(nIDEvent==DEF_UPDATEDBALLINFO_TIMER)	//更新数据库里的相关信息;
	{
        UpdateDbAllInfo();
		g_SpeedAlarmLock.Lock();
		theApp.m_asSpeedAlarmInfo.RemoveAll();
		theApp.m_asSpeedAlarmInfo.SetSize(0);
		if(g_DBWork.GetSpeedAlarm(theApp.m_asSpeedAlarmInfo)<0)
		{
			//AfxMessageBox("加载超速度报警信息失败");
			g_SpeedAlarmLock.Unlock();
			return;
		}
		g_SpeedAlarmLock.Unlock();
	}

	if(nIDEvent == DEF_DEVICE_CHECK_TIMER)	//检测设备状态
	{
		CheckDeviceStatus();
	}

	if(nIDEvent==DEF_CHECKSERVERIP_TIMER)
	{
		//检测服务器ip地址;
		DealWithEquimentSetTime();
	}
	
	CDialog::OnTimer(nIDEvent);
}

//更新数据库里的相关信息;
void CNetServerDlg::UpdateDbAllInfo()
{
	g_GpsAlarmInfoLock.Lock();
	g_GmemoryDataManager.LoadGpsAlarmInfo();
	g_GpsAlarmInfoLock.Unlock();

	g_EquimentUserLock.Lock();
	g_GmemoryDataManager.LoadEquimentUser();
	g_EquimentUserLock.Unlock();

	theApp.m_AllEquimentInfoLock.Lock();
	int iRet=g_DBWork.GetEquimentInfo(theApp.m_asEquimentInfo);
	theApp.m_AllEquimentInfoLock.Unlock();
	

}

//心跳的检测;
void CNetServerDlg::DoCheckHeatBeat()
{
	g_ClientManagerLock.Lock();	
	int  iCount=theApp.m_Clientmanager.GetUserCount();
	for(int  iLoop=0;iLoop<iCount;iLoop++)
	{
		ClintNode *pNode=theApp.m_Clientmanager.GetClientNodeFromId(iLoop);
		if(pNode==NULL)
		{
		}
		else
		{
			if(pNode->iRecvHeartbeatCount>10)
			{//连接断开;
				closesocket(pNode->fd);
				
				this->m_SocketServer.m_ReadThread.DeletNetNode(pNode->fd);
				char *p= new char[30];
				if(p)
				{
					CString  szuser=pNode->UserID;
					sprintf(p,"%s",szuser);
					//  g_ClientManagerLock.Unlock();
					theApp.m_pMainWnd->SendMessage(WM_DISCONNECT,(int)p,szuser.GetLength());
					//  return;
				//	g_ClientManagerLock.Lock();
					theApp.m_Clientmanager.DeletNode(pNode->fd);
				//	g_ClientManagerLock.Unlock();
				}
				else
				{
				//	g_ClientManagerLock.Lock();
				//	theApp.m_Clientmanager.DeletNode(pNode->fd);
				//	g_ClientManagerLock.Unlock();   
				}
				
				
			}
			else
			{
				pNode->iRecvHeartbeatCount++;
			}
		}
	}
	g_ClientManagerLock.Unlock();
	
}

//处理设备设置的时间
void CNetServerDlg::DealWithEquimentSetTime()
{
}

void CNetServerDlg::WriteGpsDateToDb(CGPSData *pDate)
{
	if(!pDate)
		return;
	COleDateTime  nowTime;
	nowTime.ParseDateTime(pDate->Time);
	if(nowTime.GetStatus()==COleDateTime::valid)
	{
		//		2007080
		CString szTable;
		szTable.Format("GPSDATA%s",nowTime.Format("%Y%m%d"));	
		g_DBWork.CreateTable(szTable);
		g_DBWork.UpdataGPSData(szTable,*pDate);
		g_DBWork.UpdataGPSNewestData(*pDate);  //最好能加个时间比较的  20070712
	}
	

}

//开始执行db存储的线程
BOOL CNetServerDlg::StartDbThread(void *p)
{
	CWinThread*  pThread=NULL;
	pThread=NULL;
//	pThread=::AfxBeginThread(GPSDBDateWriteThread,(LPVOID)this);  //启动一个监听线程，来处理数据
//	if(FAILED(pThread))
//		return  FALSE;
	return  TRUE;

}

LRESULT CNetServerDlg::OnDoPIC(WPARAM wparam,LPARAM lParam)
{
	
	return S_OK;
	int  ipIC1=(int)wparam;
	int  ipIC2=(int)lParam;
	CString  szTemp;
	szTemp.Format("%d%d",ipIC1,ipIC2);
	CEquimentInfo  info;
	CString   szEquimentID;
	if(g_DBWork.GetEquimentInfoID(szTemp,szEquimentID,TRUE)<1)
	{	
		return S_FALSE;
	}
//	AfxMessageBox(szEquimentID);
    g_PicMessageLock.Lock();
	COleDateTime  now=COleDateTime::GetCurrentTime();
	CString  sztime;
	sztime.Format("%s",now.Format("%Y-%m-%d %H:%M:%S"));
	CPicMessage  picinfo;
	picinfo.szEquimentid=szEquimentID;
	picinfo.szTime=sztime;
	picinfo.szMessage.Format("PIC-%s",szTemp);
	theApp.m_asPicMessage.Add(picinfo);

	g_PicMessageLock.Unlock();
   
	return S_OK;
}

BOOL CNetServerDlg::IsMineCar(CGPSData* pData)
{
	CString strEquipmentID = pData->szEquimentID;
	int nCarType = -1;
	int nIsRun = -1;
	m_DBWork.GetEquipmentType(strEquipmentID, nCarType, nIsRun);
	if (nCarType == 1)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

LRESULT CNetServerDlg::OnGPSDATA(WPARAM wparam, LPARAM lParam)
{
	//更新数据
	CGPSData* pPreData = NULL;
	CGPSData* pGpsData = (CGPSData*)lParam;

	CString strInfo;

	//过滤掉设备ID为空的数据
	CString strEquipmentID = pGpsData->szEquimentID;
	strEquipmentID.TrimLeft();
	strEquipmentID.TrimRight();
	if (strEquipmentID.IsEmpty())
	{
		return S_FALSE;
	}

	//过滤掉非运矿车数据
	if (!IsMineCar(pGpsData))
	{
		return S_FALSE;
	}

	COleDateTime current = COleDateTime::GetCurrentTime();
	COleDateTime dataTime;
	dataTime.ParseDateTime(pGpsData->Time);
	COleDateTimeSpan span = current - dataTime;
	int nSeconds = abs(span.GetDays() * 3600 * 24 + span.GetHours() * 3600 + span.GetMinutes() * 60 + span.GetSeconds());
	if (nSeconds > 3600)
	{
		return S_FALSE;
	}

	map<CString, CGPSData*>::iterator itr1 = m_mapGPSData.find(strEquipmentID);
	if (itr1 == m_mapGPSData.end())
	{
		m_mapGPSData.insert(pair<CString, CGPSData*>(strEquipmentID, pGpsData));
		SlowDownInfo slInfo;
		strcpy_s(slInfo.szBeginTime, 20, pGpsData->Time);
		slInfo.nExceptionNum = 0;
		m_mapExceptionInfo.insert(pair<CString, SlowDownInfo>(strEquipmentID, slInfo));
	} 
	else
	{
		pPreData = new CGPSData();
		memcpy(pPreData, m_mapGPSData[strEquipmentID], sizeof(CGPSData));
		//pPreData = m_mapGPSData[strEquipmentID];
		m_mapGPSData.erase(itr1);

		//m_mapGPSData[strEquipmentID] = pGpsData;
		m_mapGPSData.insert(pair<CString, CGPSData*>(strEquipmentID, pGpsData));

		//过滤掉重复数据
		if (0 == strcmp(pPreData->Time, pGpsData->Time))
		{
			if (NULL != pPreData)
			{
				delete pPreData;
				pPreData = NULL;
			}
			return S_FALSE;
		}
	}

	//超速报警，不仅是区域内超速进行报警，在区域外，如果运矿车超过速度35则报警，为排除终端gps漂移产生的速度影响，超过40码的速度不进行报警。
	float fSpeed = pGpsData->fSpeed * 1.852;
	if (fSpeed >= 35 && fSpeed <= 40)
	{
		strInfo.Format("%s 超速报警：设备[%s]速度[%.2f]已经超过标准速度[%.2f]", 
			pGpsData->Time, pGpsData->szEquimentID, fSpeed, 35.0);
		AreaLog(strInfo.GetBuffer(0));

		//发送短信
		//SendMessageToEquipemnt(pGpsData->szEquimentID, 4, strInfo.GetBuffer(0));

		//记录事件
		CEventInfo info;
		strcpy(info.EQUIPMENTID, pGpsData->szEquimentID);
		info.TYPE = 1;
		strcpy(info.BEGINTIME, pGpsData->Time);
		//strcpy(info.AREANAME, szLayerName.GetBuffer(0));
		info.SPEED = fSpeed;
		info.SpeedLimit = 35;

		m_DBWork.AddEventInfo(info);
	}

	//区域判断
	map<CString, CAreaInfo>::iterator itr;
	for (itr = m_mapAreaInfo.begin(); itr != m_mapAreaInfo.end(); itr ++)
	{
		CString strAreaID = itr->first;
		CAreaInfo area = itr->second;
		CMapXLayer layer = m_mapLayer[strAreaID];

		BOOL IsPreIn = FALSE;
		BOOL IsIn = FALSE;
		CString szLayerName = area.NAME;

		if (NULL == pPreData) //第一次接收到数据
		{
			IsPointInLayer(m_Map, pGpsData->fLongitude, pGpsData->fLatitude, szLayerName, IsIn);
		} 
		else //不是第一次接收到数据
		{
			IsPointInLayer(m_Map, pPreData->fLongitude, pPreData->fLatitude, szLayerName, IsPreIn);
			IsPointInLayer(m_Map, pGpsData->fLongitude, pGpsData->fLatitude, szLayerName, IsIn);
		}

		//第一次收到数据且进入区域就报警
		//不是第一次收到数据，只有第一次进入区域才报警，后面收到数据不报警
		//if((NULL == pPreData && IsIn == TRUE) || (NULL != pPreData && IsPreIn == FALSE && IsIn == TRUE) )
		if(IsIn == TRUE)
		{
			strInfo.Format("%s 设备[%s]已经进入区域[%s]", pGpsData->Time, pGpsData->szEquimentID, szLayerName.GetBuffer(0));
			AreaLog(strInfo.GetBuffer(0));

			if (area.TYPE == 0) //测速区域
			{
				// 1节=1海里/小时=1.852公里/小时
				float fSpeed = pGpsData->fSpeed * 1.852;

				if (fSpeed >= area.SPEED)
				{
					strInfo.Format("%s 超速报警：设备[%s]已经进入区域[%s]，速度[%f]已经超过设置速度[%f]", 
						pGpsData->Time, pGpsData->szEquimentID, szLayerName.GetBuffer(0), fSpeed, area.SPEED);
					AreaLog(strInfo.GetBuffer(0));

					//发送短信
					//SendMessageToEquipemnt(pGpsData->szEquimentID, 4, strInfo.GetBuffer(0));

					//记录事件
					CEventInfo info;
					strcpy(info.EQUIPMENTID, pGpsData->szEquimentID);
					info.TYPE = 1;
					strcpy(info.BEGINTIME, pGpsData->Time);
					strcpy(info.AREANAME, szLayerName.GetBuffer(0));
					info.SPEED = fSpeed;
					info.SpeedLimit = area.SPEED;

					m_DBWork.AddEventInfo(info);
				}
			}
			else if (area.TYPE == 1) //盲区提醒
			{
				CheckBlindArea(pGpsData->szEquimentID, szLayerName);
			}
		}

		//怠工提醒
		if (area.TYPE == 2) //休息区域
		{
			if (IsIn == FALSE)
			{
				CheckSlowDown(pPreData, pGpsData);
			}
		}

	}

	if (NULL != pPreData)
	{
		delete pPreData;
		pPreData = NULL;
	}

	return S_OK;
}

//得到两点之间的距离;
double GetDistance(CMapX &Map,double x1, double y1, double x2, double y2)
{
	Map.SetMapUnit(miUnitMeter);
	if(x1<1||y1<1||x2<1||y2<1)
		return  99999;
	double  iDistance=Map.Distance(x1,y1,x2,y2);
	Map.SetMapUnit(miUnitMile);
	return  iDistance;
}

//盲区检查
//盲区提醒，并不是一进区域就报警，而是在区域内如果有两辆车相距过近（暂定为50米），则通知两辆车附近有车请小心慢行。
void CNetServerDlg::CheckBlindArea(CString strEquipmentID, CString strLayerName)
{
	CGPSData* pGpsData = m_mapGPSData[strEquipmentID];

	map<CString, CGPSData*>::iterator itr;
	for (itr = m_mapGPSData.begin(); itr != m_mapGPSData.end(); itr ++)
	{
		CString strEID = itr->first;
		CGPSData* pData = m_mapGPSData[strEID];
		if (strEID != strEquipmentID)
		{
			BOOL IsIn = FALSE;
			IsPointInLayer(m_Map, pData->fLongitude, pData->fLatitude, strLayerName, IsIn);
			if (TRUE == IsIn)
			{
				double dDistance = GetDistance(m_Map, pGpsData->fLongitude, pGpsData->fLatitude,
					pData->fLongitude, pData->fLatitude);
				if (dDistance <= 100)
				{
					CString strInfo;
					strInfo.Format("%s 盲区报警：设备[%s %s]与设备[%s %s]已经进入区域[%s]，且两者距离[%.2f]小于等于设定距离[%.2f]", 
						pGpsData->Time, strEquipmentID, pGpsData->szEquimentID, strEID, pData->szEquimentID, strLayerName.GetBuffer(0), dDistance, 100.0);
					AreaLog(strInfo.GetBuffer(0));
				}
			}
		}
	}
}

//怠工提醒
void CNetServerDlg::CheckSlowDown(CGPSData* pPreData, CGPSData* pGpsData)
{
	if (NULL == pPreData)
	{
		return;
	}

	CString strEquipmentID = pPreData->szEquimentID;
	double dDistance = GetDistance(m_Map, pPreData->fLongitude, pPreData->fLatitude,
		pGpsData->fLongitude, pGpsData->fLatitude);
	if (dDistance <= 10)
	{
		COleDateTime beginTime, endTime;
		beginTime.ParseDateTime(m_mapExceptionInfo[strEquipmentID].szBeginTime);
		endTime.ParseDateTime(pGpsData->Time);
		CTimeSpan span = endTime - beginTime;
		int nSeconds = span.GetDays() * 86400 + span.GetHours() * 3600 + span.GetMinutes() * 60 + span.GetSeconds();
		if (nSeconds > 600)
		{
			CString strInfo;
			strInfo.Format("%s 怠工提醒：设备[%s]在[%s] - [%s]这段时间内停车异常", 
				pGpsData->Time, pGpsData->szEquimentID, m_mapExceptionInfo[strEquipmentID].szBeginTime, pGpsData->Time);
			AreaLog(strInfo.GetBuffer(0));

			//发送短信
			//SendMessageToEquipemnt(pGpsData->szEquimentID, 4, strInfo.GetBuffer(0));

			//记录事件
			CEventInfo info;
			strcpy(info.EQUIPMENTID, pGpsData->szEquimentID);
			info.TYPE = 2;
			strcpy(info.BEGINTIME, m_mapExceptionInfo[strEquipmentID].szBeginTime);
			strcpy(info.ENDTIME, pGpsData->Time);

			m_DBWork.AddEventInfo(info);
		}
	}
	else
	{
		strcpy_s(m_mapExceptionInfo[strEquipmentID].szBeginTime, 20, pGpsData->Time);
		m_mapExceptionInfo[strEquipmentID].nExceptionNum = 0;
	}
}


CMapXLayer  CNetServerDlg::CreateLayer(CMapX &Map, CString szLayerName)
{	
	CString  szLog;
	CMapXLayer   layer;
	CMapXLayerInfo  LayerInfo;
	CMapXFields    Fields;

	try
	{
		layer=Map.GetLayers().Item((LPCTSTR)szLayerName);
	}
	catch (...) 
	{//图层不存在;

		try
		{
			//图层信息
			if(!LayerInfo.CreateDispatch(LayerInfo.GetClsid(),NULL))
			{//

				//	return  FALSE;
			}
			if(!Fields.CreateDispatch(Fields.GetClsid()))
			{

				//	return  FALSE;
			}
			CString  szName="NAME";
			LPCTSTR lcsTmp = (LPCTSTR)szName;
			VARIANT FieldLength;              
			FieldLength.vt=VT_I2;
			FieldLength.iVal =30;
			long iwidth = 30;
			Fields.AddStringField((LPCTSTR)lcsTmp,iwidth,FALSE);
			VARIANT vFlds;
			vFlds.vt = VT_DISPATCH;
			vFlds.pdispVal = Fields.m_lpDispatch;

			LayerInfo.SetType(miLayerInfoTypeTemp);
			LayerInfo.AddParameter("Name",COleVariant(szLayerName));
			LayerInfo.AddParameter("Fields",vFlds);
			//	LayerInfo.AddParameter("图元",);
			//加入图层;
			layer=Map.GetLayers().Add(LayerInfo,0);
			//设置画图图层的属性;
			Map.GetLayers().SetAnimationLayer(layer);//设置为动态图层
			layer.GetLabelProperties().SetPosition(miPositionBC);  //标猪的位置
			layer.GetLabelProperties().SetOverlap(true);//over  write
			layer.GetLabelProperties().GetStyle().SetTextFontColor( miColorRed);
			//Style.SymbolVectorSize 
			//	CMapxTextFont
			layer.GetLabelProperties().GetStyle().SetSymbolVectorSize(9);
			//	COleFont fontl=layer.GetLabelProperties().GetStyle().GetTextFont();
			//	CY  ccy;
			//	ccy.int64=100000;
			//	fontl.SetSize(ccy);
			//	layer.SetOverrideStyle(TRUE);
			layer.SetEditable(TRUE);
			layer.SetZoomLayer(FALSE);
			layer.SetAutoLabel(FALSE);	  //自动标猪;
			//	Map.SetAutoRedraw(FALSE);    //
			//	Map.SetRedrawInterval(3000);
		}
		catch (COleDispatchException  *e )
		{

			return  layer;
		}
		catch(...)
		{
			//			AfxMessageBox("catch  cratw");
			return  layer;
		}
		return  layer;
	}
	return layer;
}

int CNetServerDlg::IsPointInLayer(CMapX &Map,double x, double y, CString szLayerName,BOOL  &ISIN)
{
	CMapXLayer  ly;
	CMapXFeatures  fts;
	CMapXPoint  pt;
	ISIN=FALSE;
	try
	{
		if(!(pt.CreateDispatch(pt.GetClsid(),NULL)))
			return  -1;
		pt.Set(x,y);
		ly=Map.GetLayers().Item(szLayerName);
		fts =ly.SearchAtPoint(pt, miSearchResultRegion);
		if(fts.GetCount()>0)
			ISIN=TRUE;
		else
			ISIN=FALSE;
	}
	catch (...)
	{
		return  -1;
	}

	return 1;
}

//void CNetServerDlg::SendMessageToEquipemnt(char* szEquipmentID, int nCommandID, char* szMessage)
//{
//	CGTQEquipment GTQEQUIMENT;
//	CGpsCommand* pSendInfo = new  CGpsCommand();
//	if(pSendInfo == NULL)
//		return;
//
//	sprintf(pSendInfo->EquimentId, "%s", szEquipmentID);
//	pSendInfo->iCOmmandID = nCommandID;
//	COleDateTime olesendTime = COleDateTime::GetCurrentTime();
//	CString szPws;
//	szPws.Format("%s", olesendTime.Format("%H%M%S"));
//	sprintf(pSendInfo->PWS, "%s", szPws);
//	sprintf(pSendInfo->Param, "%s", szMessage);
//	
//	//发送数据
//	GTQEQUIMENT.SendTQGPSCommand(szEquipmentID, pSendInfo);
//}

void CNetServerDlg::CheckDeviceStatus()
{
	map<std::string, zxEdipper> mapEdippers;
	theApp.m_pDBHelper->GetEDippers(mapEdippers);
	map<std::string, zxEdipper>::iterator itr;
	for (itr = g_mapEdippers.begin(); itr != g_mapEdippers.end(); itr ++)
	{
		std::string strDeviceID = itr->first;
		if (mapEdippers.find(strDeviceID) != mapEdippers.end())
		{
			if (itr->second.nIsRun != mapEdippers[strDeviceID].nIsRun)
			{
				bool bResult = NotifyDeviceStatusChange(strDeviceID, mapEdippers[strDeviceID].nIsRun, mapEdippers[strDeviceID].nHaltReason);
				if (bResult)
				{
					itr->second.nIsRun = mapEdippers[strDeviceID].nIsRun;
					itr->second.nHaltReason = mapEdippers[strDeviceID].nHaltReason;
				}
			}
		} 
	}
}

bool CNetServerDlg::NotifyDeviceStatusChange(std::string strDeviceID, int nIsRun, int nHaltReason)
{
	CommonMessage commonMessage;
	commonMessage.set_type(DEVICE_STATUS_CHANGE_NOTIFICATION);

	DeviceStatusChangeNotification notification;
	notification.set_device_id(strDeviceID);
	if (nIsRun == 1)
	{
		notification.set_status(RUNNING);
	} 
	else
	{
		notification.set_status(CLOSED);
	}
	
	notification.set_type(nHaltReason);

	*commonMessage.mutable_device_status_change_notification() = notification;

	bool bResult = true;

	boost::mutex::scoped_lock lock(g_mapClients_mutex);
	std::map<string, zxClient>::iterator itr = g_mapClients.find(strDeviceID);
	if (itr != g_mapClients.end())
	{
		m_NetNodeLock.Lock();
		bResult = SendCommonMessage(itr->second.nFD, commonMessage);
		m_NetNodeLock.Unlock();
	}

	return bResult;
}

bool CNetServerDlg::SendCommonMessage(int fd, CommonMessage commMessage)
{
	int size = commMessage.ByteSize();
	void *buffer = malloc(size);
	commMessage.SerializeToArray(buffer, size);

	int nDataLen = size;
	nDataLen = htonl(nDataLen);

	char* szBuffer = new char[4 + size];
	memset(szBuffer, 0x00, 4 + size);
	memcpy(szBuffer, (char*)&nDataLen, 4);
	memcpy(szBuffer + 4, buffer, size);

	int nSentLen = send(fd, szBuffer, 4 + size, 0);
	if (nSentLen == 4 + size)
	{
		return true;
	}
	else
	{
		return false;
	}
}



void CNetServerDlg::OnBnClickedExit()
{
	// TODO: Add your control notification handler code here
	if(MessageBox( TEXT("是否确认停止系统"), TEXT("警告"), MB_OKCANCEL) != IDOK)
	{
		return;
	}

	m_SocketServer.EndService();

	this->KillTimer(DEF_DB_WEB_TIMER);
	this->KillTimer(DEF_DB_SMS_TIMER);
	this->KillTimer(DEF_CHECKSERVERIP_TIMER);
	this->KillTimer(DEF_UPDATEDBALLINFO_TIMER);
	this->KillTimer(DEF_HEARTBEAT_TIMER);
	this->KillTimer(DEF_DEVICE_CHECK_TIMER);

	this->SetWindowText("GPS网络服务器---未启动");
}

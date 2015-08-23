// NetReadThread.h: interface for the NetReadThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETREADTHREAD_H__7C971012_27DC_42C9_9A82_39A710022C0A__INCLUDED_)
#define AFX_NETREADTHREAD_H__7C971012_27DC_42C9_9A82_39A710022C0A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/*******************************************************/
/*负责网络数据的读取                                   */
/*******************************************************/

#include "Thread.h"
#include "NetField.h"
#include <afxcoll.h>
#include <afxmt.h>

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#include "msg.pb.h"
using namespace zaoxun;

#include "Common.h"

#include "GPSDataProcessThread.h"
#include "CommandProcessThread.h"

enum
{
	E_COMMAND_ID_REGISTER_REQ   = 1001,
	E_COMMAND_ID_REGISTER_ACK   = 1002,
	E_COMMAND_ID_CHECK_REQ      = 1003,
	E_COMMAND_ID_CHECK_ACK      = 1004,
	E_COMMAND_ID_OILMASS_REQ    = 1005,
	E_COMMAND_ID_OILMASS_ACK    = 1006,
	E_COMMAND_ID_OUTPUT_REQ     = 1007,
	E_COMMAND_ID_OUTPUT_ACK     = 1008,
	E_COMMAND_ID_SPEEDLIMIT_MSG = 1009,
	E_COMMAND_ID_SHORT_MESSAGE  = 1010,
	E_COMMAND_ID_GPSDATA        = 1011
};

class NetReadThread : public Thread  
{
public:
	void ReleaseBuf(CNetNode *pNetData);
	void ClearBadData(CNetNode *pNetData);

	
	BOOL SendToWorkThread(CNetNode *pNetData, int  iLen, int  fd);
	
	virtual BOOL Start(int fd);
	void DeletNetNode(int  fd);
	CNetNode * SearchNetNode(int  fd);  //返回和fd对应的NetNode对象;
	int RecvNetData(CNetNode  *pNetNode);  //用来处理网络数据的接受;
	BOOL ClealFd(int  fd);
	int AddAllSockToFD(fd_set   &read_fd);	
	void SetListenFd(int  fd);
	virtual  void Run();
	NetReadThread();  
	virtual ~NetReadThread();

	void SendCommonMessage(int fd, CommonMessage commMessage);

	bool CreateTable(string strTableName);
	bool AddGPSData(GPSData gpsData);

	bool ProcessAudioFileData(AudioFileData request);
	bool ProcessAudioRecord(string strFilePath, AudioFileData request);

	bool ProcessCaptureFileData(CaptureFileData request);
	bool ProcessCaptureRecord(string strFilePath, CaptureFileData request);

	void GetNearestEdippers(string strDeviceID, std::vector<EDipperInfo>& vctEdippers);
	void ForwardMessage(int fd, CommonMessage message);

	void ProcessUnloadRequest(CommonMessage commMessage);

	void ProcessLoginRequest(std::string strDeviceID, LoginResponse& response);

	void ProcessLoadCompleteNotification(CommonMessage commMessage);

	bool PushMaterialsInfo();

	bool PushEdippersInfo();

	bool BroadcastMessage(CommonMessage msg);

	bool GetCurrentDumpingArea(std::string strDeviceID, zxArea& area);

	bool AddTaskRecord(zxTaskRecord taskRecord);

	void ProcessUnloadResponse(CommonMessage commMessage);

	bool ProcessCancelLoadRequest(CommonMessage commMessage);
	bool ProcessCancelLoadResponse(CommonMessage commMessage);
	bool ProcessRejectLoadRequest(CommonMessage commMessage);
	bool ProcessRejectLoadResponse(CommonMessage commMessage);

	bool ProcessExitNotification(CommonMessage commMessage);
	bool UpdateDeviceRunningStatus(string strDeviceID, int nStatus, int nReason);
	bool AddRunningRecord(string strDeviceID, int nStatus, int nReason, int nAreaID, char* szDriverID);

	bool ProcessCheckRequest(int fd, CommonMessage commMessage);
	bool ProcessLoadRequest(CommonMessage commMessage);
	bool ProcessLoadResponse(CommonMessage commMessage);

	bool CreateTaskRecordTable(string strTableName);

	void GetClientInfoFromDeviceID(std::string strDeviceID, zxClient& client);

private:
	int  m_fd;   //listen  Scoket;
	BOOL  m_bRun;
	CObArray    m_asNetNode;  //保存接受到的当前活动socket 连接
	//CMutex      m_NetNodeLock; 
	CString GetBufToStr(BYTE* pIn, int len);  
	int DoNetRecvError(int iError);
	BOOL IsRightMsgHead(NetMsgHead  &netHead);
	void SendNetMsg(CNetNode  *pNetData); //传递网络数据给workThread;

	std::ofstream m_audioFile;
	string   m_strAudioFilePath;

	std::ofstream m_captureFile;
	string   m_strCaptureFilePath;

	CGPSDataProcessThread m_gpsDataThread;
	CCommandProcessThread m_commnadThread;
};

#endif // !defined(AFX_NETREADTHREAD_H__7C971012_27DC_42C9_9A82_39A710022C0A__INCLUDED_)

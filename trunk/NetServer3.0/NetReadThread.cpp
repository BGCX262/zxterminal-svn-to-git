// NetReadThread.cpp: implementation of the NetReadThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetServer.h"
#include "NetReadThread.h"
#include "WinSock.h"
#include "Log.h"
#include "NetField.h"
#include "ThreadMsgManager.h"
#include "MemoryPool.h"
#include "Resource.h"

#include "fstream"
#include <algorithm>
using namespace std;

#include "msg.pb.h"
using namespace zaoxun;
//using namespace message;

#include "MapXReplace.h"

#include "google/protobuf/stubs/common.h"
#include "google/protobuf/io/coded_stream.h"
#include "controlled_module_ex.hpp"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//typedef std::pair<std::string, double> PAIR;  
//
//bool cmp_by_value(PAIR& lhs, PAIR& rhs) {  
//	return lhs.second < rhs.second;  
//} 
//
//struct CmpByValue {  
//	bool operator()(const PAIR& lhs, const PAIR& rhs) {  
//		return lhs.second < rhs.second;  
//	}  
//};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern  void      g_Log(CString szLog);
extern  CThreadMsgManager  g_ThreadMsgManager;

extern CMutex      m_NetNodeLock; 
extern  CMutex    g_MemPoolMutexLock;
extern  CMemoryPool   g_MemPool;  //用的一个线程池;
extern  CNetServerApp   theApp;
extern  CMutex  g_ClientManagerLock;

extern std::map<string, std::vector<string>> g_mapRequestTable;

extern std::map<string, zxClient> g_mapClients;
extern boost::mutex g_mapClients_mutex;

extern std::map<string, zxEdipper> g_mapEdippers;

extern std::map<std::string, zxMineCar> g_mapMineCars;

extern std::map<std::string, zxTaskRecord> g_mapTaskRecords;

extern std::map<std::string, GPSData> g_mapGPSData;
extern boost::mutex  g_mapGPSData_mutex;

extern std::map<int, CzxArea> g_mapAreas;

char* MBToUTF8(vector<char>& pu8, const char* pmb, int mLen)  
{  
	// convert an MBCS string to widechar   
	int nLen = MultiByteToWideChar(CP_ACP, 0, pmb, mLen, NULL, 0);  

	WCHAR* lpszW = NULL;  
	try  
	{  
		lpszW = new WCHAR[nLen];  
	}  
	catch(bad_alloc &memExp)  
	{  
		return NULL;  
	}  

	int nRtn = MultiByteToWideChar(CP_ACP, 0, pmb, mLen, lpszW, nLen);  

	if(nRtn != nLen)  
	{  
		delete[] lpszW;  
		return NULL;  
	}  
	// convert an widechar string to utf8  
	int utf8Len = WideCharToMultiByte(CP_UTF8, 0, lpszW, nLen, NULL, 0, NULL, NULL);  
	if (utf8Len <= 0)  
	{  
		return NULL;  
	}  
	pu8.resize(utf8Len);  
	nRtn = WideCharToMultiByte(CP_UTF8, 0, lpszW, nLen, &*pu8.begin(), utf8Len, NULL, NULL);  
	delete[] lpszW;  

	if (nRtn != utf8Len)  
	{  
		pu8.clear();  
		return NULL;  
	} 

	char* pStr = new char[pu8.size() + 1];
	copy(pu8.begin(), pu8.end(), pStr);
	pStr[pu8.size()]=0;
	return pStr;  
}  

NetReadThread::NetReadThread()
{
	//zaoxun::CGPSData gpsData;
	//gpsData.set_deviceid("13524952322");
	//gpsData.set_time("20131113 12:12:12");
	//gpsData.set_longitude("111.11");
	//gpsData.set_latitude("222.22");
	//gpsData.set_speed("35.5");
	//gpsData.set_elevation("1100");
	//gpsData.set_direction("1");

	//AddGPSData(gpsData);
}

NetReadThread::~NetReadThread()
{

}

void NetReadThread::Run()
{
	CString  szTempBegin="NetReadThread::Run  Begin";
	g_Log(szTempBegin);

	if(m_fd<0)
	{
		CString   szLog;
		szLog.Format("无效的socket描述符:%d  return ",m_fd);
		g_Log(szLog);

		CString  szTempend="NetReadThread::Run()  end";
		g_Log(szTempend);
		return;
	}

	fd_set   read_fd;
	FD_ZERO(&read_fd);
	timeval  tv;
	tv.tv_sec=2;
	tv.tv_usec=0;
	if(FD_ISSET(m_fd,&read_fd))
		FD_SET(m_fd,&read_fd);
	int iCount=-1;    
	while ( /*this->m_bRun*/  !this->m_bStopThread)
	{
		//读取数据
		AddAllSockToFD(read_fd);   //把所有有效的socket加入read_fd;
        iCount=select(m_fd,&read_fd,0,0,&tv);
		if(iCount==0)
		{//当前没有可以操作的socket
			Sleep(1000);
			CString  szLog("当前当前没有可以操作的socket连接,Sleep(1000),continue");
			g_Log(szLog);
			continue;
		}
		if(iCount==SOCKET_ERROR)
		{
			//当前有个错误不能正确的操作;
			int  iError =WSAGetLastError();
			CString  szLog;
			szLog.Format("发生了一个错误%d  continue",iError);
			g_Log(szLog);
			continue;
		}
		if(iCount>0)
		{
			//当前有可以发生操作的socket;
			if(FD_ISSET (m_fd,&read_fd))
			{
				//当前有新的连接;
				//完成scoket 的接受;
				struct  sockaddr_in  addr;
				int iaddrlen=sizeof(addr);
				int  newfd = accept(m_fd, (sockaddr *)&addr, &iaddrlen);
				if(newfd == INVALID_SOCKET )
				{//发生了错误
					int iError = WSAGetLastError();
				//	continue;
				}
				else
				{
					// 接收缓冲区
					int nRecvBuf = 64 * 1024;//设置为64K
					setsockopt(newfd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
					//发送缓冲区
					int nSendBuf = 64 * 1024;//设置为64K
					setsockopt(newfd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));

					//接受连接并且保存；
					CNetNode   *pNetNode  =new  CNetNode(newfd);	
					m_NetNodeLock.Lock();
					this->m_asNetNode.Add((CObject *)pNetNode); 					
					CString  szLog;
					szLog.Format("接受了一个新的连接,IP=%s,iPort=%d,sockIndex=%d",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),newfd);
					g_Log(szLog);
					m_NetNodeLock.Unlock();
			        g_ClientManagerLock.Lock();
					theApp.m_Clientmanager.AddNode(newfd,addr.sin_port,inet_ntoa(addr.sin_addr),"");  //用用户管理;
					g_ClientManagerLock.Unlock();
				}
				//end;
			}

			for(UINT iLoop=0;  iLoop<(read_fd.fd_count);iLoop++)
			{
				//有新的网络数据到来;			
				if(FD_ISSET(read_fd.fd_array[iLoop],&read_fd))
				{
					//接收并且处理网络数据;	
					if(read_fd.fd_array[iLoop]==(SOCKET )m_fd)
					{
						continue;
					}

                    CNetNode* pRecvNode = this->SearchNetNode(read_fd.fd_array[iLoop]);
					if(!pRecvNode)
					{
						continue;
					}

					if(this->RecvNetData(pRecvNode) < 0)
					{
						//接受数据的时候有错误发生了，连接断开;
						g_ClientManagerLock.Lock();
						CString szuser, szIp, szTime;
						theApp.m_Clientmanager.GetInfo(pRecvNode->fd, szuser, szIp, szTime);  //用用户管理;
						g_ClientManagerLock.Unlock();
						if(!szuser.IsEmpty())
						{
                             char *p= new char[30];
							 if(p)
							 {
								 sprintf(p,"%s",szuser);
							     theApp.m_pMainWnd->SendMessage(WM_DISCONNECT, (int)p, szuser.GetLength());
							 }
						}

						g_ClientManagerLock.Lock();
						theApp.m_Clientmanager.DeletNode(pRecvNode->fd);  //用用户管理;
						g_ClientManagerLock.Unlock();

						this->DeletNetNode(pRecvNode->fd);

					}
					else
					{
						//正确接受到了数据;检测并且传递给workthred ;
						SendNetMsg(pRecvNode);
					}
				}				
			}
		}
	}

	m_NetNodeLock.Lock();
	int icount = m_asNetNode.GetCount();
	for(int  i=0;i<icount;i++)
	{
		CNetNode* pNode=(CNetNode *)m_asNetNode.GetAt(i);
		if(pNode)
		{
			if(pNode->pBuf!=NULL)
			{//归还内存;			  
				this->ReleaseBuf(pNode);
			}

			delete   pNode;
			pNode=NULL;
		}						
	}	
	
	m_asNetNode.RemoveAll();
	m_asNetNode.SetSize(0);

	m_NetNodeLock.Unlock();

	CString  szTempend="NetReadThread::Run()  end";
	g_Log(szTempend);
}

void NetReadThread::SetListenFd(int fd)
{
	this->m_fd=fd;
}


void NetReadThread::SendNetMsg(CNetNode  *pNetData)
{
	if(!pNetData)
	{
		return;
	}

	int isize = sizeof(NetMsgHead);
	while(pNetData->iBufLen >= pNetData->iTotalMsgLen) //是不是达到消息的最小程度了;
	{
		CString szLog;
		szLog.Format("while(pNetData->iBufLen>=pNetData->iTotalMsgLen %d", pNetData->iBufLen);
		g_Log(szLog);

		//当前的包达到最小长度了;
		NetMsgHead* phead = (NetMsgHead*)(pNetData->pBuf);
		if(!phead)
		{
			return;
		}

		NetMsgHead head;
		head.BeginHead = phead->BeginHead;
		head.Version = ntohl(phead->Version);
		head.MsgType = phead->MsgType;
		head.MainMsgType = (MIANMSGTYPE)ntohl(phead->MainMsgType);
		head.SubMsgType = (SUBMSGTYPE)ntohl(phead->SubMsgType);
		head.NetMsgLen = ntohl(phead->NetMsgLen);
		head.ContentLen = ntohl(phead->ContentLen);
		head.EndHead = phead->EndHead;
		if(this->IsRightMsgHead(head))
		{
			//合法的协议头
			pNetData->iTotalMsgLen=head.NetMsgLen;
			if(pNetData->iBufLen>= pNetData->iTotalMsgLen) 
			{
				//消息已经接受完整;   
				if(SendToWorkThread(pNetData, head.NetMsgLen, pNetData->fd))
				{
					//处理完成后调整NetNode 的接收空间 ,并且重新设置各个参数;	
					//pNetData->ClaerNodeData();
				}		
			}
			else
			{
				//消息还没有接收完成;
				break;
			}
		}
		else
		{
			//不合法的协议头; 
			ClearBadData(pNetData); 
		}
	}

	return;
}

int NetReadThread::AddAllSockToFD(fd_set& read_fd)
{
	FD_ZERO(&read_fd);

	m_NetNodeLock.Lock();

	if(!FD_ISSET(m_fd, &read_fd))
	{
		FD_SET(m_fd, &read_fd);
	}

	for(int iLoop = 0; iLoop < this->m_asNetNode.GetSize(); iLoop ++)
	{
		CNetNode* pNode = (CNetNode*)m_asNetNode.GetAt(iLoop);
		if(!pNode)
		{
			continue;
		}

		if(!FD_ISSET(pNode->fd, &read_fd))
		{
			FD_SET(pNode->fd, &read_fd);
		}
	}

	m_NetNodeLock.Unlock();

	return  0;
}

BOOL NetReadThread::ClealFd(int fd)
{
	m_NetNodeLock.Lock();
	int iCount  =this->m_asNetNode.GetSize();
	for(int i=0;i<iCount;i++)
	{
		CNetNode * pNode=(CNetNode *)m_asNetNode.GetAt(i);
		if(!pNode)
			continue;
		if(pNode->fd==fd)
		{
			m_asNetNode.RemoveAt(i);
			break;
		}
	}
	m_NetNodeLock.Unlock();
	return  TRUE;

}

//这个函数专门用来处理网络数据的接收
//int NetReadThread::RecvNetData(CNetNode  *pNetNode)
//{
//	if(!pNetNode)
//	{
//		return 0;
//	}
//
//	int iLen = 1024;//测试用的 
//
//	g_MemPoolMutexLock.Lock();
//	if(!pNetNode->pBuf)
//	{
//		pNetNode->pBuf = g_MemPool.GetBuf();
//		ZeroMemory(pNetNode->pBuf, pNetNode->iTotalLen);
//	}
//	g_MemPoolMutexLock.Unlock();
//
//
//	if(pNetNode->pBuf==NULL)
//	{
//		//不能申请内存;
//		closesocket(pNetNode->fd);
//		return -1;
//	}
//
//	int iRecv = recv(pNetNode->fd, &(pNetNode->pBuf[pNetNode->iBufLen]), pNetNode->GetFreeBufLen(), 0);
//
//	if(iRecv == 0)
//	{
//		//网络连接断开；
//		closesocket(pNetNode->fd);
//		return  -1;
//	}
//
//	if(iRecv == SOCKET_ERROR || iRecv < 0)
//	{
//		//出现错误
//		int iError = DoNetRecvError(WSAGetLastError());
//
//		CString szLog;
//		szLog.Format("一个连接断开socket = %d, 发生了错误%d", pNetNode->fd, WSAGetLastError());
//		g_Log(szLog);
//		
//		if(iError == -1)
//		{
//			szLog.Empty();
//			szLog.Format("一个连接断开socket = %d  ,发生了错误%d  return -1", pNetNode->fd, WSAGetLastError());
//			g_Log(szLog);
//			return  -1;
//		}
//
//		if(iError == 2)
//		{
//			//系统缓冲区不足
//            //处理缓冲区  
//			return  0;
//		}
//
//		if(iError == 0)
//		{
//			return 0;
//		}
//	}
//
//	if(iRecv > 0)
//	{
//		//正确接收到数据;处理对应的CNetNode 对象;
//		pNetNode->SetBufLen(iRecv);
//		TRACE(pNetNode->pBuf);
//
//		CRegisterRequest request;
//		request.ParseFromString(pNetNode->pBuf);
//
//		std::string strDeviceID = request.deviceid();
//		char szDeviceID[20];
//		strcpy(szDeviceID, strDeviceID.c_str());
//		TRACE(szDeviceID);
//	}
//
//	return 0 ;
//}

void SaveToFile(GPSData data)
{
	std::ofstream  fs("./GPSData.txt", std::ios::app | std::ios::out);	

	std::string strContent = "";
	strContent = data.device_id() + "|" + data.time() + "|" + data.longitude() + "|" + data.latitude() + "|" + data.speed() + "|" + data.elevation() + "|" + data.direction() + "|\n";

	fs << strContent << std::endl;
	fs.close();
}

void SaveCommandToFile(int nCommandID, std::string strDeviceID)
{
	std::ofstream  fs("./Command.txt", std::ios::app | std::ios::out);	
	fs << nCommandID << "|" << strDeviceID << "|" << std::endl;
	fs.close();
}



bool NetReadThread::BroadcastMessage(CommonMessage msg)
{
	bool bResult = true;
	std::map<std::string, zxClient>::iterator itr;
	for (itr = g_mapClients.begin(); itr != g_mapClients.end(); itr ++)
	{
		SendCommonMessage(itr->second.nFD, msg);
	}

	return true;
}

bool NetReadThread::PushMaterialsInfo()
{
	std::vector<zxMaterialInfo> vctMaterials;
	if (theApp.m_pDBHelper->GetMaterials(vctMaterials))
	{
		CommonMessage commMessage;
		commMessage.set_type(MATERIALS_NOTIFICATION);

		MaterialsNotification response;
	
		for (int i = 0; i < (int)vctMaterials.size(); i ++)
		{
			Material material;
			material.set_english_name(vctMaterials[i].szEnglishName);
			material.set_chinese_name(vctMaterials[i].szChineseName);
			*(response.add_material()) = material;
		}

		*commMessage.mutable_materials_notification() = response;

		return BroadcastMessage(commMessage);
	} 
	else
	{
		return false;
	}
	
	return true;
}

bool NetReadThread::PushEdippersInfo()
{
	std::map<std::string, zxEdipper> mapEdippers;
	if (theApp.m_pDBHelper->GetEDippers(mapEdippers))
	{
		CommonMessage commMessage;
		commMessage.set_type(EDIPPERINFO_NOTIFICATION);

		EDipperInfoNotification response;

		std::map<std::string, zxEdipper>::iterator itr;
		for (itr = mapEdippers.begin(); itr != mapEdippers.end(); itr ++)
		{
			EDipperInfo edipper;
			edipper.set_device_id(itr->second.szDeviceID);
			edipper.set_device_no(itr->second.szDeviceName);
			edipper.set_longitude(itr->second.dLongitude);
			edipper.set_latitude(itr->second.dLatitude);
			*(response.add_infos()) = edipper;
		}

		*commMessage.mutable_edipperinfo_notification() = response;

		return BroadcastMessage(commMessage);
	} 
	else
	{
		return false;
	}
}

int NetReadThread::RecvNetData(CNetNode  *pNetNode)
{
	if(!pNetNode)
	{
		return 0;
	}

	int iLen = 1024;//测试用的 

	g_MemPoolMutexLock.Lock();
	if(!pNetNode->pBuf)
	{
		pNetNode->pBuf = g_MemPool.GetBuf();
		ZeroMemory(pNetNode->pBuf, pNetNode->iTotalLen);
	}
	g_MemPoolMutexLock.Unlock();


	if(pNetNode->pBuf==NULL)
	{
		//不能申请内存;
		closesocket(pNetNode->fd);
		return -1;
	}

	int nLen = 0;
	

	int iRecv = recv(pNetNode->fd, (char*)&nLen, 4, 0);
	nLen = ntohl(nLen);

	if(iRecv == 0 || nLen == 0)
	{
		//网络连接断开；
		closesocket(pNetNode->fd);
		return  -1;
	}

	if(iRecv == SOCKET_ERROR || iRecv < 0)
	{
		//出现错误
		int iError = DoNetRecvError(WSAGetLastError());

		CString szLog;
		szLog.Format("一个连接断开socket = %d, 发生了错误%d", pNetNode->fd, WSAGetLastError());
		g_Log(szLog);

		if(iError == -1)
		{
			szLog.Empty();
			szLog.Format("一个连接断开socket = %d  ,发生了错误%d  return -1", pNetNode->fd, WSAGetLastError());
			g_Log(szLog);
			return  -1;
		}

		if(iError == 2)
		{
			//系统缓冲区不足
			//处理缓冲区  
			return  0;
		}

		if(iError == 0)
		{
			return 0;
		}
	}

	char szBuffer[32 * 1024];
	//memset(szBuffer, 0x00, nLen);

	iRecv = recv(pNetNode->fd, szBuffer, nLen, 0);
	//szBuffer[iRecv] = '\0';

	if(iRecv == 0)
	{
		//delete szBuffer;
		//szBuffer = NULL;

		//网络连接断开；
		closesocket(pNetNode->fd);
		return  -1;
	}

	if(iRecv == SOCKET_ERROR || iRecv < 0)
	{
		//delete szBuffer;
		//szBuffer = NULL;

		//出现错误
		int iError = DoNetRecvError(WSAGetLastError());

		CString szLog;
		szLog.Format("一个连接断开socket = %d, 发生了错误%d", pNetNode->fd, WSAGetLastError());
		g_Log(szLog);

		if(iError == -1)
		{
			szLog.Empty();
			szLog.Format("一个连接断开socket = %d  ,发生了错误%d  return -1", pNetNode->fd, WSAGetLastError());
			g_Log(szLog);
			return  -1;
		}

		if(iError == 2)
		{
			//系统缓冲区不足
			//处理缓冲区  
			return  0;
		}

		if(iError == 0)
		{
			return 0;
		}
	}

	if(iRecv > 0)
	{
		//正确接收到数据;处理对应的CNetNode 对象;
		//pNetNode->SetBufLen(iRecv);
		//TRACE(szBuffer);

		CommonMessage commMessage;
		//commMessage.ParseFromString(szBuffer);
		typedef unsigned char uint8_t;
		google::protobuf::io::CodedInputStream cis(reinterpret_cast<uint8_t*>(szBuffer), nLen);
		cis.SetTotalBytesLimit(nLen, nLen);
		if (!commMessage.ParseFromCodedStream(&cis))
		{
			return -1;
		}

		MessageType type = commMessage.type();

		switch (type)
		{
		case LOGIN_REQUEST:
			{
				LoginRequest request = commMessage.login_request();

				std::string strDeviceID = request.device_id();
				char szDeviceID[20];
				strcpy(szDeviceID, strDeviceID.c_str());
		
				CommonMessage commMessage;
				commMessage.set_type(LOGIN_RESPONSE);

				LoginResponse response;

				ProcessLoginRequest(strDeviceID, response);
				*commMessage.mutable_login_response() = response;

				SendCommonMessage(pNetNode->fd, commMessage);

				zxClient client;
				memset(&client, 0x00, sizeof(zxClient));

				{
					boost::mutex::scoped_lock lock(g_mapClients_mutex);

					std::map<std::string, zxClient>::iterator itr = g_mapClients.find(strDeviceID);
					if (itr != g_mapClients.end())
					{
						g_mapClients.erase(itr);
					} 

					zxClient client;
					memset(&client, 0x00, sizeof(zxClient));
					strcpy(client.szDeviceID, szDeviceID);
					std::string strDeviceName = "";
					GetClientInfoFromDeviceID(strDeviceID, client);

					char szTime[20];
					memset(szTime, 0x00, 20);
					time_t tNow = time(0);
					struct tm* pt = localtime(&tNow);
					sprintf(szTime, "%d-%02d-%02d %02d:%02d:%02d", pt->tm_year + 1900, pt->tm_mon + 1, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec);
					strcpy(client.szLoginTime, szTime);

					g_mapClients.insert(std::pair<std::string, zxClient>(strDeviceID, client));
				}

				
				theApp.m_pMainWnd->SendMessage(WM_CONNECT, (WPARAM)&client, 1);	

				//push edippers information
				//PushEdippersInfo();
				//push materials information
				//PushMaterialsInfo();
				
				break;
			}

		case CHECK_REQUEST:
			{
				ProcessCheckRequest(pNetNode->fd, commMessage);

				break;
			}

		case OIL_QUANTITY_REQUEST:
			{
				//OilQuantityRequest request = commMessage.oil_quantity_request();

				//std::string strDeviceID = request.device_id();
				//char szDeviceID[20];
				//strcpy(szDeviceID, strDeviceID.c_str());
				//TRACE(szDeviceID);
				//TRACE("\n");

				//CommonMessage commMessage;
				//commMessage.set_type(OIL_QUANTITY_RESPONSE);

				//OilQuantityResponse response;
				//response.set_device_id(strDeviceID);
				//response.set_quantity(2.5);

				//*commMessage.mutable_oil_quantity_response() = response;

				//SendCommonMessage(pNetNode->fd, commMessage);

				break;
			}

		case PRODUCTION_REQUEST:
			{
				ProductionRequest request = commMessage.production_request();

				std::string strDeviceID = request.device_id();
				int nPeriod = request.period();
				char szDeviceID[20], szPeriod[10];
				strcpy(szDeviceID, strDeviceID.c_str());
				sprintf(szPeriod, "%d", nPeriod);
				TRACE(szDeviceID);
				TRACE(" ");
				TRACE(szPeriod);
				TRACE("\n");

				CommonMessage commMessage;
				commMessage.set_type(PRODUCTION_RESPONSE);

				ProductionResponse response;
				response.set_device_id(strDeviceID);
				response.set_period(nPeriod);
				response.set_quantity(3.5);

				*commMessage.mutable_production_response() = response;

				SendCommonMessage(pNetNode->fd, commMessage);

				break;
			}

		case GPS_DATA:
			{
				//GPSData request = commMessage.gps_data();

				////std::string strDeviceID = request.device_id();
				////char szDeviceID[20];
				////strcpy(szDeviceID, strDeviceID.c_str());
				////TRACE(szDeviceID);
				////TRACE("\n");

				////SaveCommandToFile(GPS_DATA, strDeviceID);
				////SaveToFile(request);

				//AddGPSData(request);

				//std::map<std::string, GPSData>::iterator itr = g_mapGPSData.find(request.device_id());
				//if (itr != g_mapGPSData.end())
				//{
				//	g_mapGPSData.erase(itr);
				//} 
				//g_mapGPSData.insert(std::pair<std::string, GPSData>(request.device_id(), request));

				//HWND hWndRcv = ::FindWindow(NULL, "RMTServer");
				//if (hWndRcv != NULL) 
				//{
				//	CGPSData GpsData;
				//	strcpy(GpsData.szEquimentID, request.device_id().c_str());
				//	GpsData.fLongitude = atof(request.longitude().c_str());
				//	GpsData.fLatitude = atof(request.latitude().c_str());
				//	GpsData.fSpeed = atof(request.speed().c_str());
				//	GpsData.iDirection = atoi(request.direction().c_str());
				//	GpsData.iState = 0;
				//	strcpy(GpsData.Time, request.time().c_str());
				//	GpsData.iFlag = 0;

				//	COPYDATASTRUCT cpd;
				//	cpd.dwData = 1; // 标志为Student类型
				//	cpd.cbData = sizeof(CGPSData);
				//	cpd.lpData = (PVOID)&GpsData;

				//	::SendMessage(hWndRcv, WM_COPYDATA, (WPARAM)1,(LPARAM)&cpd);
				//}

				m_gpsDataThread.postmessage(BM_USER + CMD_GPS_DATA, (boost::any)commMessage);

				break;
			}

		case AUDIO_FILE_DATA:
			{
				AudioFileData request = commMessage.audio_file_data();

				std::string strDeviceID = request.device_id();
				char szDeviceID[20];
				strcpy(szDeviceID, strDeviceID.c_str());
				TRACE(szDeviceID);
				TRACE("\n");

				SaveCommandToFile(AUDIO_FILE_DATA, strDeviceID);

				ProcessAudioFileData(request);

				Sleep(3000);

				int nResult = 1;
				//SendCommonMessage(pNetNode->fd, 5555, 4, (char*)&nResult);

				break;
			}

		case CAPTURE_FILE_DATA:
			{
				AudioFileData request = commMessage.audio_file_data();

				std::string strDeviceID = request.device_id();
				char szDeviceID[20];
				strcpy(szDeviceID, strDeviceID.c_str());
				TRACE(szDeviceID);
				TRACE("\n");

				SaveCommandToFile(AUDIO_FILE_DATA, strDeviceID);

				ProcessAudioFileData(request);

				break;
			}

		case GET_EDIPPERS_REQUEST:
			{
				GetEDippersRequest request = commMessage.get_edippers_request();
				std::string strDeviceID = request.device_id();
				std::vector<EDipperInfo> vctEdipper;
				GetNearestEdippers(strDeviceID, vctEdipper); 

				CommonMessage commResponse;
				commResponse.set_type(GET_EDIPPERS_RESPONSE);

				GetEDippersResponse response;
				response.set_device_id(strDeviceID);
				for (int i = 0; i < vctEdipper.size(); i ++)
				{
					*(response.add_edippers()) = vctEdipper[i];
				}
				
				*commResponse.mutable_get_edippers_response() = response;

				//ForwardMessage(m_fd, commMessage);
				SendCommonMessage(pNetNode->fd, commResponse);

				break;
			}


		case LOAD_REQUEST:
			{
				ProcessLoadRequest(commMessage);

				break;
			}

		case LOAD_RESPONSE:
			{
				ProcessLoadResponse(commMessage);

				break;
			}

		case LOAD_COMPLETE_NOTIFICATION:
			{
				ProcessLoadCompleteNotification(commMessage);

				break;
			}

		case UNLOAD_REQUEST:
			{
				ProcessUnloadRequest(commMessage);

				break;
			}

		case UNLOAD_RESPONSE:
			{
				ProcessUnloadResponse(commMessage);
				UnloadResponse request = commMessage.unload_response();

				ForwardMessage(m_fd, commMessage);

				break;
			}


		case CANCEL_LOAD_REQUEST:
			{
				ProcessCancelLoadRequest(commMessage);

				break;
			}

		case CANCEL_LOAD_RESPONSE:
			{
				ProcessCancelLoadResponse(commMessage);

				break;
			}

		case REJECT_LOAD_REQUEST:
			{
				ProcessRejectLoadRequest(commMessage);

				break;
			}

		case REJECT_LOAD_RESPONSE:
			{
				ProcessRejectLoadResponse(commMessage);

				break;
			}

		case EXIT_NOTIFICATION:
			{
				ProcessExitNotification(commMessage);

				break;
			}

		default:
			{
				break;
			}
		}
	}

	//delete szBuffer;
	//szBuffer = NULL;

	return 0 ;
}



bool NetReadThread::GetCurrentDumpingArea(std::string strDeviceID, zxArea& area)
{
	CMapXReplace *pMapX = new CMapXReplace;

	GPSData gpsData = g_mapGPSData[strDeviceID];
	std::map<int, CzxArea>::iterator itr;
	for (itr = g_mapAreas.begin(); itr != g_mapAreas.end(); itr ++)
	{
		int nPointNum = itr->second.m_vctAreaPositions.size();
		CMapXReplace::MapXPoint* points = new CMapXReplace::MapXPoint[nPointNum];
		for (int i = 0; i < nPointNum; i ++)
		{
			points[i].longitude = itr->second.m_vctAreaPositions[i].dLongitude;
			points[i].latitude = itr->second.m_vctAreaPositions[i].dLatitude;
		}
		CMapXReplace::MapXLayer layer(points, nPointNum);
		CMapXReplace::MapXPoint devicePoint(atof(gpsData.longitude().c_str()), atof(gpsData.latitude().c_str()));
		int nResult = pMapX->IsPointInLayer(layer, devicePoint);

		if (CMapXReplace::MapXOp_PIL_IN == nResult)
		{
			area.nAreaID = itr->second.m_nAreaID;
			strcpy(area.szAreaName, itr->second.m_szAreaName);

			if (pMapX)
			{
				delete pMapX;
				pMapX = NULL;
			}
			return true;
		}
	}

	if (pMapX)
	{
		delete pMapX;
		pMapX = NULL;
	}

	return false;
}

void NetReadThread::ProcessLoginRequest(std::string strDeviceID, LoginResponse& response)
{
	zxDevice device;
	response.set_device_id(strDeviceID);
	if (theApp.m_pDBHelper->GetDevice(strDeviceID, device))
	{
		response.set_car_no(device.szDeviceName);
		response.set_type((CarType)device.nDeviceType);

		if (device.nIsRun == 1)
		{
			response.set_run_status(RUNNING);
		} 
		else
		{
			response.set_run_status(CLOSED);
		}
		
		response.set_result(0);
	}
	else
	{
		response.set_result(-1);
	}
}

bool NetReadThread::ProcessAudioFileData(AudioFileData request)
{
	string strFileName = request.file_name();
	int nCurSeq = request.current_packet();
	int nTotal = request.total_packets();

	const char* szData = request.data().c_str();
	int nDataLen = request.data_len();

	if (nCurSeq < 1 || nTotal < 1)
	{
		return false;
	}

	if (1 == nCurSeq) //the first packet
	{
		if (m_audioFile.good())
		{
			m_audioFile.close();
		}

		m_strAudioFilePath = "./" + strFileName;
		m_audioFile.open(m_strAudioFilePath.c_str(), ios::out | ios::binary);
		if (!m_audioFile.good())
		{
			cout << "file open fail" << endl;
			return false;
		}

		m_audioFile.write(szData, nDataLen);
		if (nCurSeq == nTotal)
		{
			m_audioFile.close();

			ProcessAudioRecord(m_strAudioFilePath, request);
		}
	}
	else if (nCurSeq == nTotal)	//the last packet
	{
		if (!m_audioFile.good())
		{
			cout << "file open fail" << endl;
			return false;
		}

		m_audioFile.write(szData, nDataLen);
		m_audioFile.close();

		ProcessAudioRecord(m_strAudioFilePath, request);
	}
	else	//the middle packet
	{
		if (!m_audioFile.good())
		{
			cout << "file open fail" << endl;
			return false;
		}

		m_audioFile.write(szData, nDataLen);
	}

	return true;
}

bool NetReadThread::ProcessAudioRecord(string strFilePath, AudioFileData request)
{
	try
	{
		string str = "insert into audio_files values";
		str += "(:device_id<char[15]>, :time<char[20]>, :dest_device<char[15]>, :file_path<char[256]>)";
		otl_stream query(1, str.c_str(), *theApp.m_otlDB);

		char szDeviceID[15];
		char szTime[20];
		memset(szDeviceID, 0x00, 15);
		memset(szTime, 0x00, 20);
		strcpy(szDeviceID, request.device_id().c_str());
		strcpy(szTime, request.time().c_str());

		query << szDeviceID;
		query << szTime;
		query << request.dest_device().c_str();
		query << atof(strFilePath.c_str());

		return true;

	}
	catch (const otl_exception& e)
	{
		return false;
	}
	catch (const std::exception& e)
	{
		return false;
	}
}

bool NetReadThread::ProcessCaptureFileData(CaptureFileData request)
{
	string strFileName = request.file_name();
	int nCurSeq = request.current_packet();
	int nTotal = request.total_packets();

	const char* szData = request.data().c_str();
	int nDataLen = request.data_len();

	if (nCurSeq < 1 || nTotal < 1)
	{
		return false;
	}

	if (1 == nCurSeq) //the first packet
	{
		if (m_captureFile.good())
		{
			m_captureFile.close();
		}

		m_captureFile.open(m_strCaptureFilePath.c_str(), ios::out | ios::binary);
		if (!m_captureFile.good())
		{
			cout << "file open fail" << endl;
			return false;
		}

		m_audioFile.write(szData, nDataLen);
		if (nCurSeq == nTotal)
		{
			m_captureFile.close();

			ProcessCaptureRecord(m_strCaptureFilePath, request);
		}
	}
	else if (nCurSeq == nTotal)	//the last packet
	{
		if (!m_captureFile.good())
		{
			cout << "file open fail" << endl;
			return false;
		}

		m_captureFile.write(szData, nDataLen);
		m_captureFile.close();

		ProcessCaptureRecord(m_strCaptureFilePath, request);
	}
	else	//the middle packet
	{
		if (!m_captureFile.good())
		{
			cout << "file open fail" << endl;
			return false;
		}

		m_captureFile.write(szData, nDataLen);
	}

	return true;
}

bool NetReadThread::ProcessCaptureRecord(string strFilePath, CaptureFileData request)
{
	try
	{
		string str = "insert into audio_files values";
		str += "(:device_id<char[15]>, :time<char[20]>, :file_path<char[256]>)";
		otl_stream query(1, str.c_str(), *theApp.m_otlDB);

		char szDeviceID[15];
		char szTime[20];
		memset(szDeviceID, 0x00, 15);
		memset(szTime, 0x00, 20);
		strcpy(szDeviceID, request.device_id().c_str());
		strcpy(szTime, request.time().c_str());

		query << szDeviceID;
		query << szTime;
		query << atof(strFilePath.c_str());

		return true;
	}
	catch (const otl_exception& e)
	{
		return false;
	}
	catch (const std::exception& e)
	{
		return false;
	}
}

CNetNode * NetReadThread::SearchNetNode(int fd)
{
	m_NetNodeLock.Lock();

	int icount = m_asNetNode.GetSize();
	CNetNode* pNode = NULL;
	for(int i = 0; i < icount; i ++)
	{
		pNode=(CNetNode *)m_asNetNode.GetAt(i);
		if(pNode && pNode->fd == fd)
		{
			break;		
		}
	}	

	m_NetNodeLock.Unlock();

	return   pNode;
}

void NetReadThread::DeletNetNode(int fd)
{
	m_NetNodeLock.Lock();
	int icount = m_asNetNode.GetSize();
	CNetNode* pNode = NULL;
	for(int i = 0; i < icount; i ++)
	{
		pNode = (CNetNode *)m_asNetNode.GetAt(i);
		if(pNode && pNode->fd == fd)
		{
			if(pNode->pBuf != NULL)
			{
				//归还内存		  
				this->ReleaseBuf(pNode);
				pNode = NULL;
			}

			delete pNode;
			m_asNetNode.RemoveAt(i);
			break;			
		}						
	}

	m_NetNodeLock.Unlock();

	return;
}

BOOL NetReadThread::Start(int fd)
{  
	m_gpsDataThread.safestart();
	//m_commnadThread.safestart();

	this->m_fd = fd;
    return Thread::Start();
}

//验证消息头的正确;  -1:协议头的的格式不对;
BOOL   NetReadThread::IsRightMsgHead(NetMsgHead &netHead)
{
	if(netHead.BeginHead == (BYTE)MSGSIGN && netHead.EndHead == (BYTE)MSGSIGN)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//包相应的完整的信息包写到消息队列中，以后连接的请求多的时候可使用线程池来处理消息；
BOOL NetReadThread::SendToWorkThread(CNetNode *pNetData, int iLen, int fd)
{
	//复制内存到新的空间
	if(!pNetData->pBuf)
	{
		return  FALSE;
	}

	char *pData = NULL;
	pData = pNetData->pBuf; 
	if(!pData)
	{
		return  FALSE;
	}

	//打包数据到线程消息区域;
    ThreadMsg temp;
	temp.fd = fd;
	temp.iLen = iLen;
	temp.pBuf = pData;

	if(pNetData->iBufLen == pNetData->iTotalMsgLen)
	{
		//接受到的数据刚好是一个完整的数据包;
		pNetData->Inti();
		g_ThreadMsgManager.Add(temp);
		return  TRUE;		
	}

	if(pNetData->iBufLen > pNetData->iTotalMsgLen)
	{
		//接受到的数据长度大于一个包的长度;	      
		g_MemPoolMutexLock.Lock();

		//得到一个新的数据缓冲区;
		pNetData->pBuf = g_MemPool.GetBuf(); 
		if(pNetData->pBuf) 
		{
			//pNetData->Inti();
			ZeroMemory(pNetData->pBuf, BUFLEN);

			//移动多余的数据到新的缓冲区;
			memmove(pNetData->pBuf, (pData + iLen), pNetData->iBufLen-iLen);
			pNetData->iBufLen = pNetData->iBufLen - iLen;
			pNetData->iTotalLen = BUFLEN;
			if(pNetData->iTotalMsgLen < MSGMINLEN)
			{
				pNetData->iTotalMsgLen = MSGMINLEN;
			}		   
		}

		g_MemPoolMutexLock.Unlock();

		g_ThreadMsgManager.Add(temp);

		return  TRUE;
	}

	return TRUE;
}


//调试的时候输出内存信息;	
CString NetReadThread::GetBufToStr(BYTE* pIn, int len)
{		
	char pOut[1024];
	int outlen = 0;
	BYTE b;

	for(int i = 0; i < len; i ++)
	{
		b = pIn[i];
		pOut[outlen++] = ( b>>4) + (((b >> 4)>=10)? 0x37 : 0x30);
		pOut[outlen++] = (b & 0x0F) + (((b & 0x0F)>=10)? 0x37 : 0x30);
		pOut[outlen++] = ' ';
	}

	pOut[outlen] = 0;
	CString szTemp = pOut;
	return szTemp;
}

//处理接收数据是遇到的错误;
int NetReadThread::DoNetRecvError(int iError)
{
	switch(  iError   )
	{
	case   WSAENETDOWN  :     //套接字操作遇到了一个已死的网络。
	case   WSAENOTCONN  :     //由于套接字没有连接
	case   WSAENETRESET :     //当该操作在进行中，由于保持活动的操作检测到一个故障，该连接中断。
	case   WSAENOTSOCK :	  //不是socket连接符
	case   WSAESHUTDOWN :
	case   WSAECONNRESET :
	case   WSAECONNABORTED:
		return  -1;
//	case WSAECONNABORTED:  //由于系统缓冲区空间不足或列队已满，不能执行套接字上的操作啊混充
//		return   2;
	default:
		return  0;
	}

}

//去清除不合法的数据;
void NetReadThread::ClearBadData(CNetNode *pNetData)
{
	if(!pNetData)
		return ;

	BOOL bFind = FALSE;
	int iLoop = pNetData->iBufLen;
	if(!pNetData->pBuf)
	{
		pNetData->iBufLen=0;		
		return;
	}

	char *p = pNetData->pBuf;
	for(int i = 0; i < iLoop; i ++)
	{
		if(p[i]==(char )0x24    &&  i!=0 )
		{
			//去除错误的数据;
			//pNetData->iBufLen-=i;
			char* newp = new char[pNetData->iBufLen];
			if(newp == NULL)
			{
				//归还缓冲区;
				this->ReleaseBuf(pNetData);
				pNetData->iBufLen = 0;
				return;
			}

			memcpy(newp,pNetData->pBuf,pNetData->iBufLen);
			memmove(pNetData->pBuf,newp+i,pNetData->iBufLen-i);
            pNetData->iBufLen-=i;
			delete  [] newp;
			bFind=TRUE;
			return;
		}
	}
	if(!bFind)
	{
		this->ReleaseBuf(pNetData);
		pNetData->iBufLen=0;

	}
}

//
void NetReadThread::ReleaseBuf(CNetNode *pNetData)
{
	g_MemPoolMutexLock.Lock();
	g_MemPool.ReleaseBuf(pNetData->pBuf);
	pNetData->pBuf=NULL;
	g_MemPoolMutexLock.Unlock();

}

bool NetReadThread::CreateTaskRecordTable(string strTableName)
{
	//string str = "show tables like '" + strTableName + "'";
	string str = "select `TABLE_NAME` from `INFORMATION_SCHEMA`.`TABLES` where `TABLE_SCHEMA`='netdb' and `TABLE_NAME`='" + strTableName + "'";
	otl_stream query(1, str.c_str(), *theApp.m_otlDB);

	if (query.eof())
	{
		string str = "create table " + strTableName;
		str +=  "(device_id VARCHAR(32), device_name VARCHAR(32), material INT(4), driver_id VARCHAR(32), driver_name VARCHAR(32), loading_device_id VARCHAR(32), loading_device_name VARCHAR(32), loading_driver_id VARCHAR(64), loading_driver_name VARCHAR(64), loading_area_id INT(11), loading_area_name VARCHAR(32), dumping_area_id INT(11), dumping_area_name VARCHAR(32), dumping_time VARCHAR(32) NULL DEFAULT NULL, loading_time VARCHAR(32) NULL DEFAULT NULL, distance DOUBLE(12,0) NULL DEFAULT NULL, target_device_id VARCHAR(32) NULL DEFAULT NULL, target_device_name VARCHAR(32) NULL DEFAULT NULL)";

		otl_stream query(1, str.c_str(), *theApp.m_otlDB);
	}

	return true;
}

bool NetReadThread::CreateTable(string strTableName)
{
	//string str = "show tables like '" + strTableName + "'";
	string str = "select `TABLE_NAME` from `INFORMATION_SCHEMA`.`TABLES` where `TABLE_SCHEMA`='netdb' and `TABLE_NAME`='" + strTableName + "'";
	otl_stream query(1, str.c_str(), *theApp.m_otlDB);

	if (query.eof())
	{
		string str = "create table " + strTableName;
		str += "(device_id varchar(15), time varchar(20) , longitude double, latitude double, elevation double, direction int, speed double)"; 
		otl_stream query(1, str.c_str(), *theApp.m_otlDB);
	}

	return true;
}

bool NetReadThread::AddGPSData(GPSData gpsData)
{
	try
	{
		//add record to gps current table
		string str = "select * from gps_current where device_id = :device_id<char[15]>";
		otl_stream query(1, str.c_str(), *theApp.m_otlDB);

		char szDeviceID[15];
		memset(szDeviceID, 0x00, 15);
		strcpy(szDeviceID, gpsData.device_id().c_str());
		query << szDeviceID;

		if (!query.eof())
		{
			//update data
			string str2 = "update gps_current set time = :v_time<char[20]>, longitude = :v_longitude<double>, latitude = :v_latitude<double>, elevation = :v_elevation<double>, direction = :v_direction<int>, speed = :v_speed<double> where device_id = :v_device_id<char[15]>";
			otl_stream query(1, str2.c_str(), *theApp.m_otlDB);

			char szDeviceID[15];
			char szTime[20];
			memset(szDeviceID, 0x00, 15);
			memset(szTime, 0x00, 20);
			strcpy(szDeviceID, gpsData.device_id().c_str());
			strcpy(szTime, gpsData.time().c_str());

			query << szTime;
			query << atof(gpsData.longitude().c_str());
			query << atof(gpsData.latitude().c_str());
			query << atof(gpsData.elevation().c_str());
			query << atoi(gpsData.direction().c_str());
			query << atof(gpsData.speed().c_str());
			query << szDeviceID;
		} 
		else
		{
			//insert data
			string str1 = "insert into gps_current values";
			str1 += "(:device_id<char[15]>, :time<char[20]>, :longitude<double>, :latitude<double>, :elevation<double>, :direction<int>, :speed<double>)";
			otl_stream query(1, str1.c_str(), *theApp.m_otlDB);

			char szDeviceID[15];
			char szTime[20];
			memset(szDeviceID, 0x00, 15);
			memset(szTime, 0x00, 20);
			strcpy(szDeviceID, gpsData.device_id().c_str());
			strcpy(szTime, gpsData.time().c_str());

			query << szDeviceID;
			query << szTime;
			query << atof(gpsData.longitude().c_str());
			query << atof(gpsData.latitude().c_str());
			query << atof(gpsData.elevation().c_str());
			query << atoi(gpsData.direction().c_str());
			query << atof(gpsData.speed().c_str());
		}


		//add record to gps daily table
		char szTableName[20];
		memset(szTableName, 0x00, 20);
		COleDateTime current = COleDateTime::GetCurrentTime();
		sprintf(szTableName, "%s", current.Format("gps_%Y%m%d"));

		CreateTable(szTableName);

		string strTableName = szTableName;
		string str3 = "insert into " + strTableName + " values";
		str3 += "(:device_id<char[15]>, :time<char[20]>, :longitude<double>, :latitude<double>, :elevation<double>, :direction<int>, :speed<double>)";
		otl_stream insertQuery(1, str3.c_str(), *theApp.m_otlDB);

		//char szDeviceID[15];
		char szTime[20];
		memset(szDeviceID, 0x00, 15);
		memset(szTime, 0x00, 20);
		strcpy(szDeviceID, gpsData.device_id().c_str());
		strcpy(szTime, gpsData.time().c_str());

		insertQuery << szDeviceID;
		insertQuery << szTime;
		insertQuery << atof(gpsData.longitude().c_str());
		insertQuery << atof(gpsData.latitude().c_str());
		insertQuery << atof(gpsData.elevation().c_str());
		insertQuery << atoi(gpsData.direction().c_str());
		insertQuery << atof(gpsData.speed().c_str());

		return true;
	}
	catch (const otl_exception& e)
	{
		return false;
	}
	catch (const std::exception& e)
	{
		return false;
	}
}

void NetReadThread::GetNearestEdippers(string strDeviceID, std::vector<EDipperInfo>& vctEdippers)
{
	boost::mutex::scoped_lock lock(g_mapGPSData_mutex);

	std::map<std::string, double> mapDistance;
	std::map<std::string, zxEdipper>::iterator itr;
	std::map<std::string, GPSData>::iterator itrData = g_mapGPSData.find(strDeviceID) ;
	if (itrData == g_mapGPSData.end())
	{
		return;
	}

	GPSData gpsData = itrData->second;
	for (itr = g_mapEdippers.begin(); itr != g_mapEdippers.end(); itr ++)
	{
		zxEdipper edipper = itr->second;
		CMapXReplace MapXReplace;
		CMapXReplace::MapXPoint p1(atof(gpsData.longitude().c_str()), atof(gpsData.latitude().c_str()));
		CMapXReplace::MapXPoint p2(itr->second.dLongitude, itr->second.dLatitude);
		double dDistance = MapXReplace.CetDistance(p1, p2);
		if (dDistance < 500)
		{
			mapDistance.insert(std::pair<std::string, double>(itr->first, dDistance));
		}
	}

	//std::sort(mapDistance.begin(), mapDistance.end());
	int i = 0;
	for (std::map<std::string, double>::iterator itr = mapDistance.begin(); itr != mapDistance.end(); itr ++)
	{
		if (g_mapEdippers[itr->first].nIsRun == 1)
		{
			if (i < 3)
			{
				EDipperInfo dipper;
				dipper.set_device_id(g_mapEdippers[itr->first].szDeviceID);
				dipper.set_device_no(g_mapEdippers[itr->first].szDeviceName);
				dipper.set_longitude(g_mapEdippers[itr->first].dLongitude);
				dipper.set_latitude(g_mapEdippers[itr->first].dLatitude);
				vctEdippers.push_back(dipper);
				i ++;
			}
		}
	}
}

void NetReadThread::ForwardMessage(int fd, CommonMessage message)
{
	int nDataLen = sizeof(message);
	send(fd, (char*)&nDataLen, 4, 0);
	send(fd, (char*)&message, nDataLen, 0);
}

void NetReadThread::ProcessLoadCompleteNotification(CommonMessage commMessage)
{
	LoadCompleteNotification request = commMessage.load_complete_notification();
	string strDeviceID = request.car_device_id();

	boost::mutex::scoped_lock lock(g_mapClients_mutex);
	if (g_mapClients.find(strDeviceID) != g_mapClients.end())
	{
		int fd = g_mapClients[strDeviceID].nFD;
		SendCommonMessage(fd, commMessage);
	}
}

void NetReadThread::ProcessUnloadRequest(CommonMessage commMessage)
{
	UnloadRequest request = commMessage.unload_request();
	string strDeviceID = request.device_id();

	zxTaskRecord taskRecord = g_mapTaskRecords[strDeviceID];

	zxArea area;
	if (GetCurrentDumpingArea(strDeviceID, area))
	{
		taskRecord.nDumpingAreaID = area.nAreaID;

		std::vector<char> vctChar;
		int mLen = -1;
		char* pStr = MBToUTF8(vctChar, area.szAreaName, mLen);
		if (NULL != pStr)
		{
			strcpy(taskRecord.szDumpingAreaName, pStr);
		}
		else
		{
			strcpy(taskRecord.szDumpingAreaName, "");
		}
	}

	char szTime[20];
	memset(szTime, 0x00, 20);
	time_t tNow = time(0);
	struct tm* pt = localtime(&tNow);
	sprintf(szTime, "%d-%02d-%02d %02d:%02d:%02d", pt->tm_year + 1900, pt->tm_mon + 1, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec);
	strcpy(taskRecord.szDumpingTime, szTime);
	
	AddTaskRecord(taskRecord);

	boost::mutex::scoped_lock lock(g_mapClients_mutex);
	if (g_mapClients.find(strDeviceID) != g_mapClients.end())
	{
		int fd = g_mapClients[strDeviceID].nFD;

		UnloadResponse response;
		response.set_device_id(strDeviceID);
		response.set_result(0);

		CommonMessage commResponse;
		commResponse.set_type(UNLOAD_RESPONSE);
		*commResponse.mutable_unload_response() = response;

		SendCommonMessage(fd, commMessage);
	}
}

void NetReadThread::ProcessUnloadResponse(CommonMessage commMessage)
{

}

bool NetReadThread::ProcessCancelLoadRequest(CommonMessage commMessage)
{
	return true;
}

bool NetReadThread::ProcessCancelLoadResponse(CommonMessage commMessage)
{
	return true;
}

bool NetReadThread::ProcessRejectLoadRequest(CommonMessage commMessage)
{
	RejectLoadRequest request = commMessage.reject_load_request();
	string carID = request.car_device_id();

	boost::mutex::scoped_lock lock(g_mapClients_mutex);
	if (g_mapClients.find(carID) != g_mapClients.end())
	{
		int fd = g_mapClients[carID].nFD;
		SendCommonMessage(fd, commMessage);
	}
	else
	{
		return false;
	}

	return true;
}

bool NetReadThread::ProcessRejectLoadResponse(CommonMessage commMessage)
{
	RejectLoadResponse request = commMessage.reject_load_response();
	string edipperID = request.edipper_device_id();

	boost::mutex::scoped_lock lock(g_mapClients_mutex);
	if (g_mapClients.find(edipperID) != g_mapClients.end())
	{
		int fd = g_mapClients[edipperID].nFD;
		SendCommonMessage(fd, commMessage);
	}
	else
	{
		return false;
	}

	return true;
}

bool NetReadThread::ProcessExitNotification(CommonMessage commMessage)
{
	ExitNotification request = commMessage.exit_notification();
	string device_id = request.device_id();
	int exit_type = request.exit_type();

	zxArea area;
	memset(&area, 0x00, sizeof(zxArea));
	theApp.m_pDBHelper->GetCurrentArea(device_id, area);

	zxDriver driver;
	memset(&driver, 0x00, sizeof(zxDriver));
	char szDriverID[32];
	memset(&szDriverID, 0x00, 32);
	theApp.m_pDBHelper->GetDriver(device_id, driver);
	itoa(driver.nDriverID, szDriverID, 10);

	UpdateDeviceRunningStatus(device_id, 0, exit_type);
	AddRunningRecord(device_id, 0, exit_type, area.nAreaID, szDriverID);

	return true;
}

bool NetReadThread::AddTaskRecord(zxTaskRecord taskRecord)
{
	try
	{
		//add task record to table
		char szTableName[20];
		memset(szTableName, 0x00, 20);
		COleDateTime current = COleDateTime::GetCurrentTime();
		sprintf(szTableName, "%s", current.Format("task_record_%Y%m"));

		CreateTable(szTableName);

		string strTableName = szTableName;
		string str3 = "insert into " + strTableName + " values";
		str3 += "(:device_id<char[32]>, :device_name<char[32]>, :material<int>, :driver_id<char[32]>, :driver_name<char[32]>, :loading_device_id<char[32]>, :loading_device_name<char[32]>, :loading_driver_id<char[64]>, :loading_driver_name<char[64]>, :loading_area_id<int>, :loading_area_name<char[32]>, :dumping_area_id<int>, :dumping_area_name<char[32]>, :dumping_time<char[32]>, :loading_time<char[32]>, :distance<double>, :target_device_id<char[32]>, :target_device_name<char[32]>)";
		otl_stream insertQuery(1, str3.c_str(), *theApp.m_otlDB);

		char szDeviceID[32];
		char szDeviceName[32];
		memset(szDeviceID, 0x00, 32);
		memset(szDeviceName, 0x00, 32);
		strcpy(szDeviceID, taskRecord.szDeviceID);
		strcpy(szDeviceName, taskRecord.szDeviceName);

		insertQuery << szDeviceID;
		insertQuery << szDeviceName;
		insertQuery << taskRecord.nMaterialID;
		insertQuery << taskRecord.szDriverID;

		std::vector<char> vctChar;
		int mLen = -1;
		char* pStr = MBToUTF8(vctChar, taskRecord.szDriverName, mLen);
		if (NULL != pStr)
		{
			strcpy(taskRecord.szDriverName, pStr);
		}
		else
		{
			strcpy(taskRecord.szDriverName, "");
		}

		insertQuery << taskRecord.szDriverName;

		insertQuery << taskRecord.szEdipperID;
		insertQuery << taskRecord.szEdipperName;
		insertQuery << taskRecord.szEdipperDriverID;
		insertQuery << taskRecord.szEdipperDriverName;
		insertQuery << taskRecord.nLoadingAreaID;
		insertQuery << taskRecord.szLoadingAreaName;

		insertQuery << taskRecord.nDumpingAreaID;
		insertQuery << taskRecord.szDumpingAreaName;

		insertQuery << taskRecord.szDumpingTime;
		insertQuery << taskRecord.szLoadingTime;

		insertQuery << taskRecord.dDistance;

		insertQuery << taskRecord.szTargetDeviceID;
		std::vector<char> vctChar2;
		int mLen2 = -1;
		char* pStr2 = MBToUTF8(vctChar2, taskRecord.szTargetDeviceName, mLen2);
		if (NULL != pStr2)
		{
			strcpy(taskRecord.szTargetDeviceName, pStr2);
		}
		else
		{
			strcpy(taskRecord.szTargetDeviceName, "");
		}
		insertQuery << taskRecord.szTargetDeviceName;

		return true;
	}
	catch (const otl_exception& e)
	{
		string s = e.stm_text;
		return false;
	}
	catch (const std::exception& e)
	{
		return false;
	}
}

bool NetReadThread::UpdateDeviceRunningStatus(string strDeviceID, int nStatus, int nReason)
{
	try
	{
		string str = "update device_info set ";
		str += "is_run = :is_run <int>, halt_reason = :halt_reason<int> where device_id = :device_id <char[15]>";
		otl_stream insertQuery(1, str.c_str(), *theApp.m_otlDB);

		char szDeviceID[15];
		memset(szDeviceID, 0x00, 15);
		strcpy(szDeviceID, strDeviceID.c_str());

		insertQuery << nStatus;
		insertQuery << nReason;
		insertQuery << szDeviceID;

		return true;
	}
	catch (const otl_exception& e)
	{
		return false;
	}
	catch (const std::exception& e)
	{
		return false;
	}
}

bool NetReadThread::AddRunningRecord(string strDeviceID, int nStatus, int nReason, int nAreaID, char* szDriverID)
{
	try
	{
		string str = "insert into running_record values";
		str += "(:device_id <char[15]>, :is_run <int>, :time<char[32]>, :halt_reason<int>, :area_id<int>, :driver_id<char[32]>)";
		otl_stream insertQuery(1, str.c_str(), *theApp.m_otlDB);

		char szDeviceID[15];
		char szTime[20];
		memset(szDeviceID, 0x00, 15);
		memset(szTime, 0x00, 20);
		strcpy(szDeviceID, strDeviceID.c_str());

		time_t tNow = time(0);
		struct tm* pt = localtime(&tNow);
		sprintf(szTime, "%d-%02d-%02d %02d:%02d:%02d", pt->tm_year + 1900, pt->tm_mon + 1, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec);

		insertQuery << szDeviceID;
		insertQuery << nStatus;
		insertQuery << szTime;
		insertQuery << nReason;

		insertQuery << nAreaID;
		insertQuery << szDriverID;

		return true;
	}
	catch (const otl_exception& e)
	{
		return false;
	}
	catch (const std::exception& e)
	{
		return false;
	}
}

bool NetReadThread::ProcessCheckRequest(int fd, CommonMessage commMessage)
{
	CheckRequest request = commMessage.check_request();

	std::string strDeviceID = request.device_id();

	zxArea area;
	memset(&area, 0x00, sizeof(zxArea));
	theApp.m_pDBHelper->GetCurrentArea(strDeviceID, area);

	zxDriver driver;
	memset(&driver, 0x00, sizeof(zxDriver));
	char szDriverID[32];
	memset(&szDriverID, 0x00, 32);
	theApp.m_pDBHelper->GetDriver(strDeviceID, driver);
	itoa(driver.nDriverID, szDriverID, 10);

	AddRunningRecord(strDeviceID, 1, 1, area.nAreaID, szDriverID);

	CommonMessage commonMessage;
	commonMessage.set_type(CHECK_RESPONSE);

	CheckResponse response;
	response.set_device_id(strDeviceID);
	response.set_status(0);

	*commonMessage.mutable_check_response() = response;

	SendCommonMessage(fd, commonMessage);

	return true;
}

void NetReadThread::SendCommonMessage(int fd, CommonMessage commMessage)
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

	send(fd, szBuffer, 4 + size, 0);
}

bool NetReadThread::ProcessLoadRequest(CommonMessage commMessage)
{
	LoadRequest request = commMessage.load_request();
	std::string strEdipperDeviceID = request.edipper_device_id();

	boost::mutex::scoped_lock lock(g_mapClients_mutex);
	if (g_mapClients.find(strEdipperDeviceID) != g_mapClients.end())
	{
		int fd = g_mapClients[strEdipperDeviceID].nFD;
		SendCommonMessage(fd, commMessage);
	}

	return true;
}

bool NetReadThread::ProcessLoadResponse(CommonMessage commMessage)
{
	LoadResponse response = commMessage.load_response();
	string strDeviceID = response.device_id();
	string strEdipperDeviceID = response.edipper_device_id();
	int nResult = response.result();

	if (1 == nResult)
	{
		if (g_mapRequestTable.find(strEdipperDeviceID) == g_mapRequestTable.end())
		{
			std::vector<string> vctCars;
			vctCars.push_back(strDeviceID);
			g_mapRequestTable.insert(pair<std::string, std::vector<std::string>>(strEdipperDeviceID, vctCars));
		} 
		else
		{
			g_mapRequestTable[strEdipperDeviceID].push_back(strDeviceID);
		}

		zxTaskRecord taskRecord;
		memset(&taskRecord, 0x00, sizeof(taskRecord));
		strcpy(taskRecord.szDeviceID, strDeviceID.c_str());
		strcpy(taskRecord.szDeviceName, g_mapMineCars[strDeviceID].szDeviceName);
		strcpy(taskRecord.szEdipperID, response.edipper_device_id().c_str());
		strcpy(taskRecord.szEdipperName, g_mapEdippers[taskRecord.szEdipperID].szDeviceName);

		zxDriver deviceDriver;
		theApp.m_pDBHelper->GetDriver(taskRecord.szDeviceID, deviceDriver);
		itoa(deviceDriver.nDriverID, taskRecord.szDriverID, 10);
		std::vector<char> vctChar2;
		int mLen2 = -1;
		char* pStr2 = MBToUTF8(vctChar2, deviceDriver.szDriverName, mLen2);
		if (NULL != pStr2)
		{
			strcpy(taskRecord.szDriverName, pStr2);
		}
		else
		{
			strcpy(taskRecord.szDriverName, "");
		}

		zxDriver edipperDriver;
		theApp.m_pDBHelper->GetDriver(taskRecord.szEdipperID, edipperDriver);
		itoa(edipperDriver.nDriverID, taskRecord.szEdipperDriverID, 10);
		std::vector<char> vctChar3;
		int mLen3 = -1;
		char* pStr3 = MBToUTF8(vctChar3, edipperDriver.szDriverName, mLen3);
		if (NULL != pStr3)
		{
			strcpy(taskRecord.szEdipperDriverName, pStr3);
		}
		else
		{
			strcpy(taskRecord.szEdipperDriverName, "");
		}

		char szTime[20];
		memset(szTime, 0x00, 20);
		time_t tNow = time(0);
		struct tm* pt = localtime(&tNow);
		sprintf(szTime, "%d-%02d-%02d %02d:%02d:%02d", pt->tm_year + 1900, pt->tm_mon + 1, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec);
		strcpy(taskRecord.szLoadingTime, szTime);

		zxArea area;
		if (GetCurrentDumpingArea(strDeviceID, area))
		{
			taskRecord.nLoadingAreaID = area.nAreaID;

			std::vector<char> vctChar;
			int mLen = -1;
			char* pStr = MBToUTF8(vctChar, area.szAreaName, mLen);
			if (NULL != pStr)
			{
				strcpy(taskRecord.szLoadingAreaName, pStr);
			}
			else
			{
				strcpy(taskRecord.szLoadingAreaName, "");
			}
		}

		taskRecord.nMaterialID = response.material_id();
		g_mapTaskRecords.insert(std::pair<std::string, zxTaskRecord>(strDeviceID, taskRecord));
	}

	boost::mutex::scoped_lock lock(g_mapClients_mutex);
	if (g_mapClients.find(strDeviceID) != g_mapClients.end())
	{
		int fd = g_mapClients[strDeviceID].nFD;
		SendCommonMessage(fd, commMessage);

		return true;
	}

	return false;
}

void NetReadThread::GetClientInfoFromDeviceID(std::string strDeviceID, zxClient& client)
{
	std::map<std::string, zxEdipper>::iterator itr = g_mapEdippers.find(strDeviceID);
	if (itr == g_mapEdippers.end())
	{
		std::map<std::string, zxMineCar>::iterator itr1 = g_mapMineCars.find(strDeviceID);
		if (itr1 != g_mapMineCars.end())
		{
			strcpy(client.szDeviceName, itr1->second.szDeviceName);
			client.nType = MINE_CAR;
		}
		else
		{
			client.nType = OTHER_CAR;
		}
	} 
	else
	{
		strcpy(client.szDeviceName, itr->second.szDeviceName);
		client.nType = E_DIPPER;
	}
}











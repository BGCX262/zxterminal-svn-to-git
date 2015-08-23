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
using namespace std;

#include "msg.pb.h"
using namespace zaoxun;
//using namespace message;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern  void      g_Log(CString szLog);
extern  CThreadMsgManager  g_ThreadMsgManager;


extern  CMutex    g_MemPoolMutexLock;
extern  CMemoryPool   g_MemPool;  //�õ�һ���̳߳�;
extern  CNetServerApp   theApp;
extern  CMutex  g_ClientManagerLock;

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
		szLog.Format("��Ч��socket������:%d  return ",m_fd);
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
		//��ȡ����
		AddAllSockToFD(read_fd);   //��������Ч��socket����read_fd;
        iCount=select(m_fd,&read_fd,0,0,&tv);
		if(iCount==0)
		{//��ǰû�п��Բ�����socket
			Sleep(1000);
			CString  szLog("��ǰ��ǰû�п��Բ�����socket����,Sleep(1000),continue");
			g_Log(szLog);
			continue;
		}
		if(iCount==SOCKET_ERROR)
		{
			//��ǰ�и���������ȷ�Ĳ���;
			int  iError =WSAGetLastError();
			CString  szLog;
			szLog.Format("������һ������%d  continue",iError);
			g_Log(szLog);
			continue;
		}
		if(iCount>0)
		{
			//��ǰ�п��Է���������socket;
			if(FD_ISSET (m_fd,&read_fd))
			{
				//��ǰ���µ�����;
				//���scoket �Ľ���;
				struct  sockaddr_in  addr;
				int iaddrlen=sizeof(addr);
				int  newfd=accept(m_fd,(sockaddr *)&addr,&iaddrlen);
				if(newfd==INVALID_SOCKET )
				{//�����˴���
					int iError=WSAGetLastError();
				//	continue;
				}
				else
				{
					//�������Ӳ��ұ��棻
					CNetNode   *pNetNode  =new  CNetNode(newfd);	
					m_NetNodeLock.Lock();
					this->m_asNetNode.Add((CObject *)pNetNode); 					
					CString  szLog;
					szLog.Format("������һ���µ�����,IP=%s,iPort=%d,sockIndex=%d",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),newfd);
					g_Log(szLog);
					m_NetNodeLock.Unlock();
			        g_ClientManagerLock.Lock();
					theApp.m_Clientmanager.AddNode(newfd,addr.sin_port,inet_ntoa(addr.sin_addr),"");  //���û�����;
					g_ClientManagerLock.Unlock();
				}
				//end;
			}

			for(UINT iLoop=0;  iLoop<(read_fd.fd_count);iLoop++)
			{
				//���µ��������ݵ���;			
				if(FD_ISSET(read_fd.fd_array[iLoop],&read_fd))
				{
					//���ղ��Ҵ�����������;	
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
						//�������ݵ�ʱ���д������ˣ����ӶϿ�;
						g_ClientManagerLock.Lock();
						CString szuser, szIp, szTime;
						theApp.m_Clientmanager.GetInfo(pRecvNode->fd, szuser, szIp, szTime);  //���û�����;
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
						theApp.m_Clientmanager.DeletNode(pRecvNode->fd);  //���û�����;
						g_ClientManagerLock.Unlock();

						this->DeletNetNode(pRecvNode->fd);

					}
					else
					{
						//��ȷ���ܵ�������;��Ⲣ�Ҵ��ݸ�workthred ;
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
			{//�黹�ڴ�;			  
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
	while(pNetData->iBufLen >= pNetData->iTotalMsgLen) //�ǲ��Ǵﵽ��Ϣ����С�̶���;
	{
		CString szLog;
		szLog.Format("while(pNetData->iBufLen>=pNetData->iTotalMsgLen %d", pNetData->iBufLen);
		g_Log(szLog);

		//��ǰ�İ��ﵽ��С������;
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
			//�Ϸ���Э��ͷ
			pNetData->iTotalMsgLen=head.NetMsgLen;
			if(pNetData->iBufLen>= pNetData->iTotalMsgLen) 
			{
				//��Ϣ�Ѿ���������;   
				if(SendToWorkThread(pNetData, head.NetMsgLen, pNetData->fd))
				{
					//������ɺ����NetNode �Ľ��տռ� ,�����������ø�������;	
					//pNetData->ClaerNodeData();
				}		
			}
			else
			{
				//��Ϣ��û�н������;
				break;
			}
		}
		else
		{
			//���Ϸ���Э��ͷ; 
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

//�������ר�����������������ݵĽ���
//int NetReadThread::RecvNetData(CNetNode  *pNetNode)
//{
//	if(!pNetNode)
//	{
//		return 0;
//	}
//
//	int iLen = 1024;//�����õ� 
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
//		//���������ڴ�;
//		closesocket(pNetNode->fd);
//		return -1;
//	}
//
//	int iRecv = recv(pNetNode->fd, &(pNetNode->pBuf[pNetNode->iBufLen]), pNetNode->GetFreeBufLen(), 0);
//
//	if(iRecv == 0)
//	{
//		//�������ӶϿ���
//		closesocket(pNetNode->fd);
//		return  -1;
//	}
//
//	if(iRecv == SOCKET_ERROR || iRecv < 0)
//	{
//		//���ִ���
//		int iError = DoNetRecvError(WSAGetLastError());
//
//		CString szLog;
//		szLog.Format("һ�����ӶϿ�socket = %d, �����˴���%d", pNetNode->fd, WSAGetLastError());
//		g_Log(szLog);
//		
//		if(iError == -1)
//		{
//			szLog.Empty();
//			szLog.Format("һ�����ӶϿ�socket = %d  ,�����˴���%d  return -1", pNetNode->fd, WSAGetLastError());
//			g_Log(szLog);
//			return  -1;
//		}
//
//		if(iError == 2)
//		{
//			//ϵͳ����������
//            //��������  
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
//		//��ȷ���յ�����;�����Ӧ��CNetNode ����;
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
	//CString strFileName = "./Command.txt";
	//CStdioFile  datafile(strFileName, CFile::modeCreate | CFile::modeReadWrite);

	std::ofstream  fs("./Command.txt", std::ios::app | std::ios::out);	
	fs << nCommandID << "|" << strDeviceID << "|" << std::endl;
	fs.close();


	//char szCommand[10];
	//sprintf(szCommand, "%d", nCommandID);
	//std::string strContent = szCommand;
	//strContent = strContent + "|" + strDeviceID + "|\n";
	//datafile.WriteString(strContent.c_str());

	//datafile.Close();
}

void NetReadThread::ProcessResponse(int fd, int nCommandID, int nLen, char* szResponse)
{
	send(fd, (char*)&nCommandID, 4, 0);
	send(fd, (char*)&nLen, 4, 0);
	send(fd, szResponse, nLen, 0);
}

int NetReadThread::RecvNetData(CNetNode  *pNetNode)
{
	if(!pNetNode)
	{
		return 0;
	}

	int iLen = 1024;//�����õ� 

	g_MemPoolMutexLock.Lock();
	if(!pNetNode->pBuf)
	{
		pNetNode->pBuf = g_MemPool.GetBuf();
		ZeroMemory(pNetNode->pBuf, pNetNode->iTotalLen);
	}
	g_MemPoolMutexLock.Unlock();


	if(pNetNode->pBuf==NULL)
	{
		//���������ڴ�;
		closesocket(pNetNode->fd);
		return -1;
	}

	int nCommandID = 0;
	int nLen = 0;
	char szBuffer[1024];
	memset(szBuffer, 0x00, 1024);
	
	int iRecv = recv(pNetNode->fd, (char*)&nCommandID, 4, 0);
	nCommandID = ntohl(nCommandID);

	if(iRecv == 0)
	{
		//�������ӶϿ���
		closesocket(pNetNode->fd);
		return  -1;
	}

	if(iRecv == SOCKET_ERROR || iRecv < 0)
	{
		//���ִ���
		int iError = DoNetRecvError(WSAGetLastError());

		CString szLog;
		szLog.Format("һ�����ӶϿ�socket = %d, �����˴���%d", pNetNode->fd, WSAGetLastError());
		g_Log(szLog);

		if(iError == -1)
		{
			szLog.Empty();
			szLog.Format("һ�����ӶϿ�socket = %d  ,�����˴���%d  return -1", pNetNode->fd, WSAGetLastError());
			g_Log(szLog);
			return  -1;
		}

		if(iError == 2)
		{
			//ϵͳ����������
			//��������  
			return  0;
		}

		if(iError == 0)
		{
			return 0;
		}
	}


	iRecv = recv(pNetNode->fd, (char*)&nLen, 4, 0);
	nLen = ntohl(nLen);
	
	if(iRecv == 0)
	{
		//�������ӶϿ���
		closesocket(pNetNode->fd);
		return  -1;
	}

	if(iRecv == SOCKET_ERROR || iRecv < 0)
	{
		//���ִ���
		int iError = DoNetRecvError(WSAGetLastError());

		CString szLog;
		szLog.Format("һ�����ӶϿ�socket = %d, �����˴���%d", pNetNode->fd, WSAGetLastError());
		g_Log(szLog);

		if(iError == -1)
		{
			szLog.Empty();
			szLog.Format("һ�����ӶϿ�socket = %d  ,�����˴���%d  return -1", pNetNode->fd, WSAGetLastError());
			g_Log(szLog);
			return  -1;
		}

		if(iError == 2)
		{
			//ϵͳ����������
			//��������  
			return  0;
		}

		if(iError == 0)
		{
			return 0;
		}
	}

	iRecv = recv(pNetNode->fd, szBuffer, nLen, 0);
	szBuffer[iRecv] = '\0';

	if(iRecv == 0)
	{
		//�������ӶϿ���
		closesocket(pNetNode->fd);
		return  -1;
	}

	if(iRecv == SOCKET_ERROR || iRecv < 0)
	{
		//���ִ���
		int iError = DoNetRecvError(WSAGetLastError());

		CString szLog;
		szLog.Format("һ�����ӶϿ�socket = %d, �����˴���%d", pNetNode->fd, WSAGetLastError());
		g_Log(szLog);

		if(iError == -1)
		{
			szLog.Empty();
			szLog.Format("һ�����ӶϿ�socket = %d  ,�����˴���%d  return -1", pNetNode->fd, WSAGetLastError());
			g_Log(szLog);
			return  -1;
		}

		if(iError == 2)
		{
			//ϵͳ����������
			//��������  
			return  0;
		}

		if(iError == 0)
		{
			return 0;
		}
	}

	if(iRecv > 0)
	{
		//��ȷ���յ�����;�����Ӧ��CNetNode ����;
		//pNetNode->SetBufLen(iRecv);
		//TRACE(szBuffer);

		switch (nCommandID)
		{
		case E_COMMAND_ID_REGISTER_REQ:
			{
				RegisterRequest request;
				request.ParseFromString(szBuffer);

				std::string strDeviceID = request.deviceid();
				char szDeviceID[20];
				strcpy(szDeviceID, strDeviceID.c_str());
				TRACE(szDeviceID);
				TRACE("\n");

				SaveCommandToFile(E_COMMAND_ID_REGISTER_REQ, strDeviceID);

				CRegisterResponse response;
				response.set_deviceid(strDeviceID);
				response.set_result(0);

				char szResponse[1024];
				memset(szResponse, 0x00, 1024);
				int nSerializeLen = -1;
				response.SerializeToArray(szResponse, nSerializeLen);

				ProcessResponse(pNetNode->fd, E_COMMAND_ID_REGISTER_ACK, nSerializeLen, szResponse);
				
				break;
			}

		case E_COMMAND_ID_CHECK_REQ:
			{
				CCheckRequest request;
				request.ParseFromString(szBuffer);

				std::string strDeviceID = request.deviceid();
				char szDeviceID[20];
				strcpy(szDeviceID, strDeviceID.c_str());
				TRACE(szDeviceID);
				TRACE("\n");


				CCheckResponse response;
				response.set_deviceid(strDeviceID);
				response.set_status(1);

				char szResponse[1024];
				memset(szResponse, 0x00, 1024);
				int nSerializeLen = -1;
				response.SerializeToArray(szResponse, nSerializeLen);

				ProcessResponse(pNetNode->fd, E_COMMAND_ID_CHECK_ACK, nSerializeLen, szResponse);
				SaveCommandToFile(E_COMMAND_ID_CHECK_REQ, strDeviceID);
				break;
			}

		case E_COMMAND_ID_OILMASS_REQ:
			{
				COilmassRequest request;
				request.ParseFromString(szBuffer);

				std::string strDeviceID = request.deviceid();
				char szDeviceID[20];
				strcpy(szDeviceID, strDeviceID.c_str());
				TRACE(szDeviceID);
				TRACE("\n");


				COilmassResponse response;
				response.set_deviceid(strDeviceID);
				response.set_quantity(2.5);

				char szResponse[1024];
				memset(szResponse, 0x00, 1024);
				int nSerializeLen = -1;
				response.SerializeToArray(szResponse, nSerializeLen);

				ProcessResponse(pNetNode->fd, E_COMMAND_ID_OILMASS_ACK, nSerializeLen, szResponse);
				SaveCommandToFile(E_COMMAND_ID_OILMASS_REQ, strDeviceID);
				break;
			}

		case E_COMMAND_ID_OUTPUT_REQ:
			{
				COutputRequest request;
				request.ParseFromString(szBuffer);

				std::string strDeviceID = request.deviceid();
				int nPeriod = request.period();
				char szDeviceID[20], szPeriod[10];
				strcpy(szDeviceID, strDeviceID.c_str());
				sprintf(szPeriod, "%d", nPeriod);
				TRACE(szDeviceID);
				TRACE(" ");
				TRACE(szPeriod);
				TRACE("\n");


				COutputResponse response;
				response.set_deviceid(strDeviceID);
				response.set_period(nPeriod);
				response.set_quantity(3.5);

				char szResponse[1024];
				memset(szResponse, 0x00, 1024);
				int nSerializeLen = -1;
				response.SerializeToArray(szResponse, nSerializeLen);

				ProcessResponse(pNetNode->fd, E_COMMAND_ID_OUTPUT_ACK, nSerializeLen, szResponse);
				SaveCommandToFile(E_COMMAND_ID_OUTPUT_REQ, strDeviceID);
				break;
			}

		case E_COMMAND_ID_GPSDATA:
			{
				zaoxun::CGPSData request;
				request.ParseFromString(szBuffer);

				std::string strDeviceID = request.deviceid();
				char szDeviceID[20];
				strcpy(szDeviceID, strDeviceID.c_str());
				TRACE(szDeviceID);
				TRACE("\n");

				SaveCommandToFile(E_COMMAND_ID_GPSDATA, strDeviceID);
				SaveToFile(request);

				AddGPSData(request);

				//CCheckResponse response;
				//response.set_deviceid(strDeviceID);
				//response.set_status(1);

				//char szResponse[1024];
				//memset(szResponse, 0x00, 1024);
				//int nSerializeLen = -1;
				//response.SerializeToArray(szResponse, nSerializeLen);

				//ProcessResponse(pNetNode->fd, E_COMMAND_ID_CHECK_ACK, nSerializeLen, szResponse);

				break;
			}

		default:
			break;
		}

		
	}

	return 0 ;
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
				//�黹�ڴ�		  
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
	this->m_fd = fd;
    return Thread::Start();
}

//��֤��Ϣͷ����ȷ;  -1:Э��ͷ�ĵĸ�ʽ����;
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



//����Ӧ����������Ϣ��д����Ϣ�����У��Ժ����ӵ�������ʱ���ʹ���̳߳���������Ϣ��
BOOL NetReadThread::SendToWorkThread(CNetNode *pNetData, int iLen, int fd)
{
	//�����ڴ浽�µĿռ�
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

	//������ݵ��߳���Ϣ����;
    ThreadMsg temp;
	temp.fd = fd;
	temp.iLen = iLen;
	temp.pBuf = pData;

	if(pNetData->iBufLen == pNetData->iTotalMsgLen)
	{
		//���ܵ������ݸպ���һ�����������ݰ�;
		pNetData->Inti();
		g_ThreadMsgManager.Add(temp);
		return  TRUE;		
	}

	if(pNetData->iBufLen > pNetData->iTotalMsgLen)
	{
		//���ܵ������ݳ��ȴ���һ�����ĳ���;	      
		g_MemPoolMutexLock.Lock();

		//�õ�һ���µ����ݻ�����;
		pNetData->pBuf = g_MemPool.GetBuf(); 
		if(pNetData->pBuf) 
		{
			//pNetData->Inti();
			ZeroMemory(pNetData->pBuf, BUFLEN);

			//�ƶ���������ݵ��µĻ�����;
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


//���Ե�ʱ������ڴ���Ϣ;	
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

//������������������Ĵ���;
int NetReadThread::DoNetRecvError(int iError)
{
	switch(  iError   )
	{
	case   WSAENETDOWN  :     //�׽��ֲ���������һ�����������硣
	case   WSAENOTCONN  :     //�����׽���û������
	case   WSAENETRESET :     //���ò����ڽ����У����ڱ��ֻ�Ĳ�����⵽һ�����ϣ��������жϡ�
	case   WSAENOTSOCK :	  //����socket���ӷ�
	case   WSAESHUTDOWN :
	case   WSAECONNRESET :
	case   WSAECONNABORTED:
		return  -1;
//	case WSAECONNABORTED:  //����ϵͳ�������ռ䲻����ж�����������ִ���׽����ϵĲ��������
//		return   2;
	default:
		return  0;
	}

}

//ȥ������Ϸ�������;
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
			//ȥ�����������;
			//pNetData->iBufLen-=i;
			char* newp = new char[pNetData->iBufLen];
			if(newp == NULL)
			{
				//�黹������;
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

bool NetReadThread::CreateTable(string strTableName)
{
	//string str = "show tables like '" + strTableName + "'";
	string str = "select `TABLE_NAME` from `INFORMATION_SCHEMA`.`TABLES` where `TABLE_SCHEMA`='netdb' and `TABLE_NAME`='" + strTableName + "'";
	otl_stream query(1, str.c_str(), theApp.m_otlDB);

	if (query.eof())
	{
		string str = "create table " + strTableName;
		str += "(MobileNo varchar(15), DataTime varchar(20) , Longitude double, Latitude double, Speed double, Elevation double, Direction int, State int)"; 
		otl_stream query(1, str.c_str(), theApp.m_otlDB);
	}

	return true;
}

bool NetReadThread::AddGPSData(zaoxun::CGPSData gpsData)
{
	try
	{
		char szTableName[20];
		memset(szTableName, 0x00, 20);
		COleDateTime current = COleDateTime::GetCurrentTime();
		sprintf(szTableName, "%s", current.Format("GPSData%Y%m%d"));

		CreateTable(szTableName);

		string strTableName = szTableName;
		string str = "insert into " + strTableName + " values";
		str += "(:MobileNo<char[15]>, :DataTime<char[20]>, :Longitude<double>, :Latitude<double>, :Speed<double>, :Elevation<double>, :Direction<int>, :State<int>)";
		otl_stream query(1, str.c_str(), theApp.m_otlDB);

		char szDeviceID[15];
		char szTime[20];
		memset(szDeviceID, 0x00, 15);
		memset(szTime, 0x00, 20);
		strcpy(szDeviceID, gpsData.deviceid().c_str());
		strcpy(szTime, gpsData.time().c_str());

		query << szDeviceID;
		query << szTime;
		query << atof(gpsData.longitude().c_str());
		query << atof(gpsData.latitude().c_str());
		query << atof(gpsData.speed().c_str());
		query << atof(gpsData.elevation().c_str());
		query << atoi(gpsData.direction().c_str());
		query << 0;

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























// NetEquipment.cpp: implementation of the CNetEquipment class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetEquipment.h"
#include "EquipmentManager.h"
#include "Net.h"
#include "Log.h"
#include "fstream"
using namespace std;
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define   MAXCONNTION   5000
#define   MAXFREEBUFFERCOUNT  1024
extern  void      g_Log(CString szLog);


#ifdef _DEBUG
int  CNetEquipment::m_NewContextBufCount=0;
int  CNetEquipment::m_NewIOCPBufCount=0;
int  CNetEquipment::m_UserBufCount=0;
#endif

int  iGPSDATECOunt=0;

void   __UDPRECVLog(CString  szLog)
{	
	CString szTemp;
	szTemp.Format("%d    %s",iGPSDATECOunt++,szLog);
	char buf[1024]={0};
	sprintf(buf,"%s",szTemp);
	//	ofstream  f;
	ofstream  f("UdpRecv.log",ios::app| ios::out);	
	f<<buf<<endl;
	f.close();
}




UINT ListenThread(LPVOID  pParmer)
{//启动一个监听闲程
	if(!pParmer)
		return  0;
	CNetEquipment  *pNetEquipment=reinterpret_cast<CNetEquipment *>(pParmer);
	if(!pNetEquipment)
		return  0;
	fd_set  read_fd;
	timeval  tv;
	tv.tv_sec =2;
	tv.tv_usec=0;


/*

	//udp socket  和完成端口连接;
    if((pNetEquipment->m_Udpfd_MTJJA)!=INVALID_SOCKET)
	{
		pNetEquipment->CreateNewEqNode(pNetEquipment->m_Udpfd_MTJJA,EQUIPMENT_MTJJA);
		
	}
	if((pNetEquipment->m_Udpfd_MTGPS)!=INVALID_SOCKET)
	{
		pNetEquipment->CreateNewEqNode(pNetEquipment->m_Udpfd_MTGPS,EQUIPMENT_MTGPS);
		
	}
	//end

*/

	while (pNetEquipment->m_beRun)
	{//	
		FD_ZERO(&read_fd);
		//HQ
		if((pNetEquipment->m_Tcpfd_HQGPS)!=INVALID_SOCKET)
		{
			if(!FD_ISSET(pNetEquipment->m_Tcpfd_HQGPS,&read_fd))
				FD_SET(pNetEquipment->m_Tcpfd_HQGPS,&read_fd);
		}
		 //TQ
		if((pNetEquipment->m_Tcpfd_TQGPS)!=INVALID_SOCKET)
		{
			if(!FD_ISSET(pNetEquipment->m_Tcpfd_TQGPS,&read_fd))
				FD_SET(pNetEquipment->m_Tcpfd_TQGPS,&read_fd);
		}
		//GTQ 20070606
		if((pNetEquipment->m_Tcpfd_GTQGPS)!=INVALID_SOCKET)
		{
			if(!FD_ISSET(pNetEquipment->m_Tcpfd_GTQGPS,&read_fd))
				FD_SET(pNetEquipment->m_Tcpfd_GTQGPS,&read_fd);
		}

		//MTGPS
		//MTGPS
		if((pNetEquipment->m_Udpfd_MTGPS)!=INVALID_SOCKET)
		{
			if(!FD_ISSET(pNetEquipment->m_Udpfd_MTGPS,&read_fd))
				FD_SET(pNetEquipment->m_Udpfd_MTGPS,&read_fd);
		}

		if(read_fd.fd_count<=0)
		{//当前没有需要tcp监听推出
			CString  szLog;
			szLog.Format("ListenThread 当前没有需要监听的TCP服务,退出监听线程");
			g_Log(szLog);
			break;
		}
		int iCount=select(pNetEquipment->m_Tcpfd_HQGPS+1,&read_fd,0,0,&tv);		
		if(iCount==0)
		{
			Sleep(1000);
			continue;
		}
		if(iCount==SOCKET_ERROR  ||  iCount<0)
		{
		
			int iError=WSAGetLastError();
			//DoError();  WSAENOTSOCK    WSAENETDOWN 就不能continue;
			CString szLog;
			szLog.Format("ListenThread  发生了错误%s",iError);
			g_Log(szLog);
			continue;
		}
		if(iCount>0)
		{//处理连接;
			//HQ_GPS
			BOOL  BCreateOK=FALSE;
			int  acceptfd;
			if(FD_ISSET(pNetEquipment->m_Tcpfd_HQGPS,&read_fd))
			{//接受一个HQGPS设备的连接;
				
				int iEquipmentTyep=EQUIPMENT_HQGPS;  //设备的类型标志;
				struct  sockaddr_in  addr;
				int iLen=sizeof(addr);
				acceptfd=accept(pNetEquipment->m_Tcpfd_HQGPS,(SOCKADDR *)&addr,&iLen);
				if(acceptfd==SOCKET_ERROR  || acceptfd<=0  )
				{
					int iError=WSAGetLastError();
					CString szLog;
					szLog.Format("ListenThread  m_Tcpfd_HQGPS   accept() 发生了错误%d",iError);
					g_Log(szLog);
					continue;
				}
				else
				{//有了个新的连接;
					pNetEquipment->CreateNewEqNode(acceptfd,EQUIPMENT_HQGPS);
				}			
			}
			//TQ
			if(FD_ISSET(pNetEquipment->m_Tcpfd_TQGPS,&read_fd))
			{//接受一个HQGPS设备的连接;
				
				int iEquipmentTyep=EQUIPMENT_TQGPS;  //设备的类型标志;
				struct  sockaddr_in  addr;
				int iLen=sizeof(addr);
				acceptfd=accept(pNetEquipment->m_Tcpfd_TQGPS,(SOCKADDR *)&addr,&iLen);
				if(acceptfd==SOCKET_ERROR  || acceptfd<=0  )
				{
					int iError=WSAGetLastError();

					CString szLog;
					szLog.Format("ListenThread  m_Tcpfd_TQGPS   accept() 发生了错误%s",iError);
					g_Log(szLog);
					continue;
				}
				else
				{//有了个新的连接;
					pNetEquipment->CreateNewEqNode(acceptfd,EQUIPMENT_TQGPS);
				}			
			}

			//GTQ  20070606
			if(FD_ISSET(pNetEquipment->m_Tcpfd_GTQGPS,&read_fd))
			{//接受一个HQGPS设备的连接;
				
				int iEquipmentTyep=EQUIPMENT_GTQGPS;  //设备的类型标志;
				struct  sockaddr_in  addr;
				int iLen=sizeof(addr);
				acceptfd=accept(pNetEquipment->m_Tcpfd_GTQGPS,(SOCKADDR *)&addr,&iLen);
				if(acceptfd==SOCKET_ERROR  || acceptfd<=0  )
				{
					int iError=WSAGetLastError();
					
					CString szLog;
					szLog.Format("ListenThread  m_Tcpfd_GTQGPS   accept() 发生了错误%s",iError);
					g_Log(szLog);
					continue;
				}
				else
				{//有了个新的连接;
					pNetEquipment->CreateNewEqNode(acceptfd,EQUIPMENT_GTQGPS);
				}			
			}
			if(FD_ISSET(pNetEquipment->m_Udpfd_MTGPS,&read_fd))
			{
				CIOCPBuff * pEuqimentBuff=pNetEquipment->AllocateBuffer(0);
				if(!pEuqimentBuff)
					continue;				
				int iLen=sizeof(pEuqimentBuff->addr);				
				int  iRet=recvfrom(pNetEquipment->m_Udpfd_MTGPS,pEuqimentBuff->buf,MAXIOCPBUFSIZE,0,(SOCKADDR  *)&(pEuqimentBuff->addr),&iLen);
				
				if(iRet>0)
				{//正确的网络数据;
					
					__UDPRECVLog(pEuqimentBuff->buf);
					pEuqimentBuff->m_wsabuf.buf=pEuqimentBuff->buf;
					pEuqimentBuff->fd=pNetEquipment->m_Udpfd_MTGPS;
                    pEuqimentBuff->m_iEuqType=EQUIPMENT_MTGPS;
					pEuqimentBuff->iMsgLen=iRet;
					pNetEquipment->DisBuffer(pEuqimentBuff);
					pNetEquipment->AddToUseBuffer(pEuqimentBuff);
				}
				if(iRet==0)
				{
					AfxMessageBox("MTGPS Is EROR");
				}
				//	FD_CLR(pNetEquipment->m_Udpfd_MTGPS,,&read_fd);
			}
			

		}		
	}
}
UINT WorkThread(LPVOID  pParmer)
{//启动一个工作闲程
	if(!pParmer)
		return  0;

	CNetEquipment  *pNetEquipment=reinterpret_cast<CNetEquipment *>(pParmer);
	if(!pNetEquipment)
		return  0;
	DWORD dwIoSize;
	bool bError=false;
	LPOVERLAPPED lpOverlapped;
	ClientContext* lpClientContext;
	CIOCPBuff *pOverlapBuff;
	HANDLE hCompletionPort =pNetEquipment->m_IOCPHANDLE ;

	
	//pOverlapPlus = CONTAINING_RECORD(lpOverlapped, MYOVERLAPPEDPLUS, m_ol);

		// Get a completed IO request.

	if(!pNetEquipment->m_beRun)
		return   0;
	while(!bError  &&  pNetEquipment->m_beRun )
	{
		pOverlapBuff=NULL;
		lpClientContext=NULL;
		
		BOOL bIORet = GetQueuedCompletionStatus(
			hCompletionPort,
			&dwIoSize,
			(LPDWORD) &lpClientContext,
			&lpOverlapped, INFINITE);
		if(!bIORet)
		{	//有错误；
			//进行处理
		   int  i=GetLastError();
		   pNetEquipment->OnRead(lpClientContext,pOverlapBuff);  //added 20070423

		}
		//得到缓冲区;
        if(!lpOverlapped) 
		{

			CString szLog;
			szLog="WorkThread  lpOverlapped==NULL return -1; ";
			g_EquimentWorkThreadLog(szLog);
			return -1;
		}
		pOverlapBuff=CONTAINING_RECORD(lpOverlapped,CIOCPBuff,m_ol);
		pOverlapBuff->iMsgLen=dwIoSize;
		if(bIORet &&  lpOverlapped  &&  pNetEquipment && lpClientContext &&pOverlapBuff)
		{//有消息需要处理;
			pNetEquipment->DoNetMsg(pOverlapBuff,lpClientContext,dwIoSize);	


		}

	}

}


extern int CreateTcpSocketServer(const  char  *szip,const int iPort);


extern  int CreateUdpScoket(const  char  *szip,const int iPort);

CNetEquipment::CNetEquipment()
{
	m_IOCPHANDLE=NULL;
	m_beRun=FALSE;
	m_iNumberOfPendlingReads=4;
	m_iMaxNumberOfFreeBuffer=MAXFREEBUFFERCOUNT;

	//addede  by tgc 20061107
	m_Tcpfd_HQGPS=INVALID_SOCKET;
	m_Tcpfd_TQGPS=INVALID_SOCKET;
	m_Tcpfd_GTQGPS=INVALID_SOCKET;
	m_Udpfd_MTJJA=INVALID_SOCKET;
	m_Udpfd_MTGPS=INVALID_SOCKET;


}

CNetEquipment::~CNetEquipment()
{

}

BOOL CNetEquipment::Start()
{

	this->m_beRun=TRUE;
	return  this->Run();

}

BOOL CNetEquipment::Run()
{
	//addede  by tgc 20061107
  
  CSystemInfo  info;
  if(info.GetIsServerStart(EQUIPMENT_HQGPS))
  {//HQ设备启动;
	  this->m_Tcpfd_HQGPS=CreateTcpSocketServer(NULL,(short)HQGPS_PORT);
	  if(m_Tcpfd_HQGPS<0)
	  {
		  //AfxMessageBox("start HQGPS error! ");
		  CString  szLog;
		  szLog.Format("start HQGPS error!");
		  g_Log(szLog);
		  return  FALSE;
	  }
  }
  if(info.GetIsServerStart(EQUIPMENT_TQGPS))
  {
	   this->m_Tcpfd_TQGPS=CreateTcpSocketServer(NULL,(short)TQGPS_PORT);

	   	if(m_Tcpfd_TQGPS<0)
		{
			  AfxMessageBox("start TQGPS error! ");
			  CString  szLog;
			  szLog.Format("tart TQGPS error!");
			  g_Log(szLog);
			  return  FALSE;
		 }

  }
  //added 20070606
  if(info.GetIsServerStart(EQUIPMENT_GTQGPS))
  {
	  this->m_Tcpfd_GTQGPS=CreateTcpSocketServer(NULL,(short)GTQGPS_PORT);
	  
	  if(m_Tcpfd_GTQGPS<0)
	  {
		  AfxMessageBox("start TGQGPS error! ");
		  CString  szLog;
		  szLog.Format("tart GTQGPS error!");
		  g_Log(szLog);
		  return  FALSE;
	  }
	  
  }
  if(info.GetIsServerStart(EQUIPMENT_MTJJA))
  {
	  this->m_Udpfd_MTJJA=CreateUdpScoket(NULL,(short)MTJJA_PORT);
	  if(m_Udpfd_MTJJA<0)
	  {
		  AfxMessageBox("start MTJJA error! ");
		  CString  szLog;
		  szLog.Format("start MTJJA error!");
		  g_Log(szLog);
		  return  FALSE;
	  }
  }

  if(info.GetIsServerStart(EQUIPMENT_MTGPS))
  {
	  this->m_Udpfd_MTGPS=CreateUdpScoket(NULL,(short)MTGPS_PORT);
	  if(m_Udpfd_MTGPS<0)
	  {
		  AfxMessageBox("start MTGPS error! ");
		  CString  szLog;
		  szLog.Format("start MTGPS error!");
		  g_Log(szLog);
		  return  FALSE;
	  }
  }

  //创建一个完成端口；
  m_IOCPHANDLE=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
  if(FAILED(m_IOCPHANDLE))
  {
	 CString  szLog;
	 szLog.Format("CreateIoCompletionPort  error!");
   	g_Log(szLog);
	  return  FALSE;  
  }
    
  if (SetListenPro()  && SetWorkPro())
	  return  TRUE;
  else
	  return FALSE;   
  m_ContextMap.RemoveAll();
  m_ContextMap.InitHashTable(MAXCONNTION);
   return  TRUE;
}

BOOL CNetEquipment::SetWorkPro()
{

	CWinThread*  pThread=NULL;
	SYSTEM_INFO  systeminfo;
	::GetSystemInfo(&systeminfo);
	for(int i=0;i<systeminfo.dwNumberOfProcessors;i++)
	{
		pThread=NULL;
		pThread=::AfxBeginThread(WorkThread,(LPVOID)this);  //启动一个工作线程，来处理数据
		if(FAILED(pThread))
			return  FALSE;
	}
	return  TRUE;
}

BOOL CNetEquipment::SetListenPro()
{
	CWinThread*  pThread=NULL;
	pThread=NULL;
	pThread=::AfxBeginThread(ListenThread,(LPVOID)this);  //启动一个监听线程，来处理数据
	if(FAILED(pThread))
		return  FALSE;
	return  TRUE;
}

BOOL CNetEquipment::AssociateIOCP(int  socket, HANDLE hCompletionPort, DWORD dwCompletionKey)
{
	if(socket<=0)
		return  FALSE;
	HANDLE h = CreateIoCompletionPort((HANDLE)socket, hCompletionPort, (DWORD)dwCompletionKey, 0);
	return h == hCompletionPort;
}

BOOL CNetEquipment::CreateNewEqNode(int fd,int iEquipmentType)
{
	//开辟新的缓冲结构并且加入管理
	ClientContext  *pClientContext=NULL;
    pClientContext=AllocateContext();
	ASSERT(pClientContext);
	if(!pClientContext)
	{	//申请新的内存失败;
		//清除新的socket；
		ReleaseSocket(fd);
		CString  szLog;
		szLog.Format("无法申请新的内存,EquipmentType =%d,fd=%d  return FALSE",iEquipmentType,fd);
		g_Log(szLog);
		return  FALSE;
	}
	//接受新的连接	  加如；连接池;
	pClientContext->fd=fd;
	pClientContext->m_EquipmentType=iEquipmentType;  //在其中加入设备的类型信息;
	if(AddClientContext(pClientContext))  //添加到连接池
	{//增加成功
		//关联一个连接到完成端口;
		if(!this->AssociateIOCP(fd,this->m_IOCPHANDLE,(DWORD)pClientContext))
		{//关联失败  ，清除相关的连接信息;

			CString  szLog;
			szLog.Format("无法创建关联");
			g_Log(szLog);
			ReleaseSocket(fd);
			DisconnectClient(pClientContext);
			ReleaseClientContext(pClientContext);
			return  FALSE;
		}
		//分配内存？？
		EnterIOLoop(pClientContext);
		int   i=0;
		CIOCPBuff *pOverlapBuff=AllocateBuffer(IOInitialize);   //刚连接消息
		if(pOverlapBuff!=NULL)
		{//成功申请到新的内存
			BOOL bSuccess = PostQueuedCompletionStatus(m_IOCPHANDLE, 0, (DWORD) pClientContext, &pOverlapBuff->m_ol);
			if ( (!bSuccess && GetLastError( ) != ERROR_IO_PENDING))
			{            
				ReleaseBuffer(pOverlapBuff);				
				DisconnectClient(pClientContext);
				ReleaseClientContext(pClientContext);
				return FALSE;
			}		
		}
		else
		{
			DisconnectClient(pClientContext);
			ReleaseClientContext(pClientContext);
		}
	}
	else
	{
		//要考虑释放资源;  added 20070428
		DisconnectClient(pClientContext);
		ReleaseClientContext(pClientContext);
		return  FALSE;	
	}
	return  TRUE;
}

void CNetEquipment::ReleaseClient(int fd)
{

}

ClientContext * CNetEquipment::AllocateContext()
{
	ClientContext  *pClientContext=NULL;
	
	if(this->m_beRun)
	{
		m_ContextMapLock.Lock();
		if(m_FreeContextList.IsEmpty())
		{//空闲的ClientContext 列表是空的； 分配新的ClientContext
            pClientContext=  new  ClientContext;
#ifdef   _DEBUG
			m_NewContextBufCount++;
			CString  szCount;
			TRACE("new  ClientContext  %d\n",m_NewContextBufCount);
#endif
		}
		else
		{//从空闲的列表中取一个
			pClientContext=(ClientContext *)m_FreeContextList.RemoveHead();
		}
		m_ContextMapLock.Unlock();
	}
	else
		return  NULL;
	
	ASSERT(pClientContext);
	if(!pClientContext)
	{//得到新的ClientContext失败;
		return  NULL;		
	}
	//处理新得到的ClientContext；
	pClientContext->m_ContextLock.Lock();
	pClientContext->m_nNumberOfPendlingIO=0;
	pClientContext->fd=0;
	pClientContext->keyid=0;
	pClientContext->m_ReadBufferMap.RemoveAll();
	pClientContext->m_SendBufferMap.RemoveAll();
	pClientContext->m_CurReadoderNune=0;
    pClientContext->m_CurSendooderNune=0;
	pClientContext->m_ContextLock.Unlock();
	return  pClientContext;
}

void CNetEquipment::DoNetMsg(CIOCPBuff *pOverlapBuff, ClientContext *pContext, DWORD dwSize)
{//处理相关消息的函数;
    if(!pOverlapBuff)
		return ;
	switch(pOverlapBuff->Getoperation())
	{

    	case   IOInitialize:
			OnInitialize(pContext,dwSize,pOverlapBuff);
			break;
		case IORead:
			OnRead(pContext,pOverlapBuff);
			break;
		case IOZeroByteRead:
			OnZeroByteRead(pContext,pOverlapBuff);
			break;
		case IOZeroReadCompleted:
			OnZeroByteReadCompleted(pContext, dwSize, pOverlapBuff);
			break;
		case IOReadCompleted:
			OnReadCompleted(pContext,dwSize,pOverlapBuff);
			break;
		default:
			ReleaseBuffer(pOverlapBuff);
			break;	
	}
}

BOOL CNetEquipment::AddClientContext(ClientContext *pContext)
{
	if(pContext!=NULL)
	{
		this->m_ContextMapLock.Lock();
		unsigned int  KEY_ID=pContext->fd;
		ClientContext *pTempContext=NULL;
		pTempContext=(ClientContext*)m_ContextMap[KEY_ID];
		if(pTempContext!=NULL)
		{
			this->m_ContextMapLock.Unlock();
			return  FALSE;
		}
		else
		{
			pContext->keyid=KEY_ID;
			m_ContextMap[KEY_ID]=pContext;
			this->m_CurAcctiveConntionNum++; //连接池的记数
			m_ContextMapLock.Unlock();
			
			return  TRUE;
		}	
	}
	return  FALSE;



}

void CNetEquipment::DisconnectClient(ClientContext *pContext)
{
	if(pContext!=NULL)
	{	
		pContext->m_ContextLock.Lock();
		BOOL bDisconnect=pContext->fd!=INVALID_SOCKET;
		pContext->m_ContextLock.Unlock();
		// If we have an active  socket close it. 
		if(bDisconnect)
		{		
			
			//
			// Remove it From m_ContextMap. 
			//
			m_ContextMapLock.Lock();
			BOOL bRet=FALSE;
			//Remove it from the m_ContextMapLock,, 
			unsigned int  KEY_ID=pContext->fd;
			if((ClientContext*)(m_ContextMap[KEY_ID])!=NULL)
			{
				bRet=m_ContextMap.RemoveKey(KEY_ID);
			//	if(bRet)
			//		m_NumberOfActiveConnections--;
			}	
			m_ContextMapLock.Unlock();			
			pContext->m_ContextLock.Lock();
			// Notify that we are going to Disconnect A client. 
	//		NotifyDisconnectedClient(pContext);
			pContext->m_ContextLock.Unlock();		
		    ReleaseSocket(pContext->fd);
			pContext->fd = INVALID_SOCKET;
			
		}

	}

			


}

BOOL CNetEquipment::ReleaseClientContext(ClientContext *pContext)
{
	BOOL bRet=FALSE;	
	if(pContext!=NULL)
	{

		int nNumberOfPendlingIO=ExitIOLoop(pContext);
		if(nNumberOfPendlingIO==0)
		{

		
		pContext->m_ContextLock.Lock();
	//	NotifyContextRelease(pContext);	
		pContext->m_ContextLock.Unlock();
		
		// Move the Context to the free context list (if Possible). 
		m_FreeContextListLock.Lock();	
		bRet=m_FreeContextList.AddHead((void*)pContext)!=NULL;	
		m_FreeContextListLock.Unlock();
		return TRUE; 	
	}
}
return FALSE; 
}

void CNetEquipment::EnterIOLoop(ClientContext *pContext)
{
	if(pContext!=NULL)
	{
		pContext->m_ContextLock.Lock();
		pContext->m_nNumberOfPendlingIO++;
		pContext->m_ContextLock.Unlock();
	}

}
int CNetEquipment::ExitIOLoop(ClientContext *pContext)
{
	int m_nNumberOfPendlingIO=0;
	if(pContext!=NULL)
	{
		pContext->m_ContextLock.Lock();
		pContext->m_nNumberOfPendlingIO--;
		m_nNumberOfPendlingIO=pContext->m_nNumberOfPendlingIO;
		pContext->m_ContextLock.Unlock();
		
	}
	return m_nNumberOfPendlingIO; 
}

CIOCPBuff * CNetEquipment::AllocateBuffer(int nType)
{

	CString   szLog  ;
	szLog.Format("AllocateBuffer begin %d",nType);
//	__AllocateBufferLog(szLog);
	CIOCPBuff  *pIOBuf=NULL;
	m_FreeBufferListLock.Lock();
	int  icount=this->m_FreeBufferList.GetCount(); //得到空闲的缓冲区的个数;
    if(!m_FreeBufferList.IsEmpty())
	{
       pIOBuf=(CIOCPBuff *)m_FreeBufferList.RemoveHead();
	}
	m_FreeBufferListLock.Unlock();
	if(pIOBuf==NULL)
	{//没有分配到新的iocpbuf；
		pIOBuf=new CIOCPBuff;

#ifdef   _DEBUG
		m_NewIOCPBufCount++;
		CString  szCount;
		TRACE("new CIOCPBuff  %d\n",m_NewIOCPBufCount);
#endif
		if(!pIOBuf)
		{
			szLog="";
			szLog="pIOBuf=NULL  return  NULL";
//			__AllocateBufferLog(szLog);
			return  NULL;
		}
	}
	if(pIOBuf)
	{//得到了新的iocpbuf
		pIOBuf->EmptyUsed();  //把使用的的数量清空;
//		pIOBuf->SetOpertion(0);
		pIOBuf->SetOpertion(nType);
		POSITION pos=NULL;
		this->m_BufferListLock.Lock();
		pos=m_BufferList.AddHead((void *)pIOBuf);
		
		if(pos==NULL)
		{//加如正在使用的内存区失败;
			delete  pIOBuf;
            pIOBuf=NULL;
			this->m_BufferListLock.Unlock();
			szLog="pos==NULL  return  NULL";
//			__AllocateBufferLog(szLog);
			return  NULL;
		}
		else
		{//加如正在使用的内存区成功
			pIOBuf->SetPosition(pos);
		}
		this->m_BufferListLock.Unlock();
		szLog="";
		szLog.Format("return pIOBuf==%d",pIOBuf );
//		__AllocateBufferLog(szLog);
		return  pIOBuf;
	}
	return  NULL;
}

BOOL CNetEquipment::ReleaseBuffer(CIOCPBuff  *pIOBuf)
{
	//TEST
//	AfxMessageBox("ReleaseBuffer");
	if(pIOBuf==NULL)
		return  FALSE;
	m_BufferListLock.Lock();
	POSITION pos=pIOBuf->GetPosition();
	if(pos==NULL)
	{

		CString  szLog="不能清除内存";
		g_Log(szLog);
		delete  pIOBuf;
		pIOBuf=NULL;
		m_BufferListLock.Unlock();
		return  FALSE;
	}
	m_BufferList.RemoveAt(pos);	
	m_BufferListLock.Unlock();
	//加入空闲的buflist
	this->m_FreeBufferListLock.Lock();
	if(m_iMaxNumberOfFreeBuffer==0  ||m_FreeBufferList.GetCount()<m_iMaxNumberOfFreeBuffer)
	{//正常加入
		pos=m_FreeBufferList.AddHead((void *)pIOBuf);
		pIOBuf->SetPosition(NULL);

	}
	else
	{
		delete pIOBuf; 
	}
	
	this->m_FreeBufferListLock.Unlock();
	pIOBuf=NULL;
	return  TRUE;
}

void CNetEquipment::OnInitialize(ClientContext *pContext, DWORD dwIoSize,CIOCPBuff *pOverlapBuff)
{

	if(!pContext  /*|| !pOverlapBuff*/)
	{

		return ;
	}
	pContext->m_ContextLock.Lock();
	this->NotifyNewConnection(pContext);
	pContext->m_ContextLock.Unlock();
//


	if(!this->SocketBeInUdpfd(pContext->fd))
		AZeroByteRead(pContext,pOverlapBuff);

	
	for(int i=0;i<m_iNumberOfPendlingReads;i++)
	{
		EnterIOLoop(pContext); // One for each Read Loop
		ARead(pContext);
	}

	

}
void CNetEquipment::NotifyNewConnection(ClientContext *pcontext)
{
	//现在没有做任何的处理;  可以加入联机时后的处理  //20070428
	return;
	
}



BOOL CNetEquipment::AZeroByteRead(ClientContext *pContext, CIOCPBuff *pOverlapBuff)
{
	if(!pContext)
		return  FALSE;
	if(pContext->fd!=INVALID_SOCKET)
	{
		if(pOverlapBuff==NULL)	
			pOverlapBuff=AllocateBuffer(IOZeroByteRead);		
		 pOverlapBuff->SetOpertion(IOZeroByteRead);
		 BOOL bSuccess = PostQueuedCompletionStatus(m_IOCPHANDLE, 0, (DWORD) pContext, &pOverlapBuff->m_ol); 	
		 if((!bSuccess && GetLastError( ) != ERROR_IO_PENDING))
		 {
			 ReleaseBuffer(pOverlapBuff);
			 DisconnectClient(pContext);
			 ReleaseClientContext(pContext);
			 return  FALSE;
		 }
	}
	else
	{
		ReleaseBuffer(pOverlapBuff);
		ReleaseClientContext(pContext);// Take care of it.
		return  FALSE;
	}
	return  TRUE;
}
BOOL CNetEquipment::ARead(ClientContext *pContext,CIOCPBuff*pOverlapBuff)
{
	
	if (pContext == NULL)
		return FALSE;
	
	if(pContext->fd!=INVALID_SOCKET )
	{
		if(pOverlapBuff==NULL) 
			pOverlapBuff=AllocateBuffer(IORead);
		pOverlapBuff->SetOpertion(IORead);
		BOOL bSuccess = PostQueuedCompletionStatus(m_IOCPHANDLE, 0, (DWORD) pContext, &pOverlapBuff->m_ol);  	
		if ( (!bSuccess && GetLastError( ) != ERROR_IO_PENDING))
		{            
			ReleaseBuffer(pOverlapBuff);
			ReleaseClientContext(pContext);
			return FALSE;
		}
		
	}else
	{
		ReleaseBuffer(pOverlapBuff);
		ReleaseClientContext(pContext);// Take care of it.
		return FALSE;
	}
	return TRUE;
}

void CNetEquipment::OnZeroByteRead(ClientContext *pContext, CIOCPBuff *pOverlapBuff)
{


	if(pContext==NULL)
	{

		return ;
	}
	DWORD dwIoSize=0;
	ULONG			ulFlags = MSG_PARTIAL;
	if(pOverlapBuff==NULL)
	{
		pOverlapBuff=AllocateBuffer(IOZeroReadCompleted);
		if(pOverlapBuff==NULL)
		{//不能申请到缓冲区
			DisconnectClient(pContext);
			ReleaseClientContext(pContext);
		}
	}
	pOverlapBuff->SetOpertion(IOZeroReadCompleted);
	pOverlapBuff->SetupZeroByteRead();
	UINT nRetVal=SOCKET_ERROR;
	if(SocketBeInUdpfd(pContext->fd))  //检查是不是udp的socket；
	{
        
	//	struct sockaddr_in  addr;
		int   ilen=sizeof(struct sockaddr_in);
		nRetVal=WSARecvFrom(pContext->fd,
			pOverlapBuff->GetWSABuffer(),
			1,
			&dwIoSize,
			&ulFlags,
			(SOCKADDR  *)&pOverlapBuff->addr,
		     &ilen,
			&pOverlapBuff->m_ol,
			NULL);

	}
	else
	{

		 nRetVal = WSARecv(pContext->fd, 
			pOverlapBuff->GetWSABuffer(),
			1,
			&dwIoSize, 
			&ulFlags,
			&pOverlapBuff->m_ol, 
			NULL);
	}

	if ( nRetVal == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) 
	{
		int iTes=WSAGetLastError();
//		if(WSAGetLastError()!=WSAENOTSOCK)
		{

		}
		ReleaseBuffer(pOverlapBuff);

		DisconnectClient(pContext);			
		ReleaseClientContext(pContext);
	}



}

void CNetEquipment::OnZeroByteReadCompleted(ClientContext *pContext, DWORD dwIoSize, CIOCPBuff *pOverlapBuff)
{

	if(pContext)
	{
		AZeroByteRead(pContext,pOverlapBuff);	
	}
}

void CNetEquipment::OnRead(ClientContext *pContext, CIOCPBuff *pOverlapBuff)
{

	if(pContext==NULL)
	{


		return;
	}
	if(pOverlapBuff==NULL)
	{
		pOverlapBuff=AllocateBuffer(IOReadCompleted);
		if(pOverlapBuff==NULL)
		{//不能得到缓冲区;
			DisconnectClient(pContext);
			ReleaseClientContext(pContext);
		}
	}
	if(pOverlapBuff!=NULL)
	{
		pOverlapBuff->SetOpertion(IOReadCompleted);
		pOverlapBuff->SetupRead();
		DWORD dwIoSize=0;
		ULONG			ulFlags = MSG_PARTIAL;
		UINT nRetVal=SOCKET_ERROR;
		if(SocketBeInUdpfd(pContext->fd))  //检查是不是udp的socket；
		{
			
		//	struct sockaddr_in  addr;
			int   ilen=sizeof(struct sockaddr_in);
			nRetVal=WSARecvFrom(pContext->fd,
				pOverlapBuff->GetWSABuffer(),
				1,
				&dwIoSize,
				&ulFlags,
				(SOCKADDR  *)&pOverlapBuff->addr,
				&ilen,
				&pOverlapBuff->m_ol,
				NULL);
			
		}
		else
		{
			nRetVal = WSARecv(pContext->fd, 
				pOverlapBuff->GetWSABuffer(),
				1,
				&dwIoSize, 
				&ulFlags,
				&pOverlapBuff->m_ol, 
				NULL);
		}


		if ( nRetVal == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) 
		{
			ReleaseBuffer(pOverlapBuff);
			DisconnectClient(pContext);
			ReleaseClientContext(pContext);
		}

	}

}

void CNetEquipment::OnReadCompleted(ClientContext *pContext, DWORD dwIoSize, CIOCPBuff *pOverlapBuff)
{//接收到新的网络数据;  //根据不同的设备类型来处理不同的数据;

	//editede 20070428
	if(dwIoSize==0 || pOverlapBuff== NULL )
	{
		
		DisconnectClient(pContext);
		ReleaseClientContext(pContext);
		ReleaseBuffer(pOverlapBuff);
		pOverlapBuff=NULL;
		closesocket(pContext->fd);
		return;
	}
	if(!pContext)
		return;
	pOverlapBuff->fd=pContext->fd;
    pOverlapBuff->m_iEuqType=pContext->m_EquipmentType;
   if(SocketBeInUdpfd(pContext->fd))
   {//接受的是udp数据;
	   
	   static   int inCount=0;
	   TRACE("iCOUnt=%d",inCount++);


	   __UDPRECVLog(pOverlapBuff->buf);
	   this->DisBuffer(pOverlapBuff);
	   ARead(pContext);   
//	   this->AddToUseBuffer(pOverlapBuff);
   }
   else
   {//接受的是tcp数据;
	    __UDPRECVLog(pOverlapBuff->buf);
	   this->DisBuffer(pOverlapBuff);
	   ARead(pContext);
   }
   this->AddToUseBuffer(pOverlapBuff);

   
}

//如果是udp端口就WsArecvfrim
BOOL CNetEquipment::SocketBeInUdpfd(int fd)
{
	if(fd==this->m_Udpfd_MTJJA  ||  fd==this->m_Udpfd_MTGPS)
		return  TRUE;
	return  FALSE;
}

//把缓冲区从队列中移除;
BOOL CNetEquipment::DisBuffer(CIOCPBuff *pIOBuf)
{
	if(pIOBuf==NULL)
		return  FALSE;
	m_BufferListLock.Lock();
	POSITION pos=pIOBuf->GetPosition();
	if(pos==NULL)
	{
		
		CString  szLog="不能清除内存";
		//		g_Log(szLog);
		m_BufferListLock.Unlock();
		return  FALSE;
	}
	m_BufferList.RemoveAt(pos);	
	m_BufferListLock.Unlock();
	return  TRUE;
}

BOOL CNetEquipment::AddToUseBuffer(CIOCPBuff *pIOBuf)
{
	CString szLog;
	if(!pIOBuf)
		return  FALSE;
	//加入工作的buflist
	POSITION pos=NULL;
	m_InUseBufferListLock.Lock();
	pos=m_InUseBufferList.AddHead((void *)pIOBuf);
#ifdef   _DEBUG

	TRACE("  AddToUseBuffer  m_InUseBufferList  count  %d\n",m_InUseBufferList.GetCount());
#endif
//edited  by tgc 20061109 

	if(pos==NULL)
	{

		ReleaseUseBuffer(pIOBuf);
		m_InUseBufferListLock.Unlock();
		return  FALSE;
		
	}
	m_InUseBufferListLock.Unlock();
	return  TRUE;
}

//释放正在使用的缓冲区;
BOOL CNetEquipment::ReleaseUseBuffer(CIOCPBuff *pIOBuf)
{
    CString   szLog;
	if(pIOBuf==NULL)
		return  FALSE;
#ifdef   _DEBUG
	
	TRACE(" ReleaseUseBuffer m_InUseBufferList  count  %d\n",m_InUseBufferList.GetCount());
#endif

//	m_InUseBufferListLock.Lock();
	POSITION pos=pIOBuf->GetPosition();
	if(pos==NULL)
	{		
		CString  szLog="不能清除内存";
		g_Log(szLog);
		delete  pIOBuf;
		pIOBuf=NULL;
//		m_InUseBufferListLock.Unlock();
		return  FALSE;
	}
//	int iCount=m_InUseBufferList.GetCount();
//	if(iCount>0)
	{
//		m_InUseBufferList.RemoveAt(pos);
	}
		
//	m_InUseBufferListLock.Unlock();
	//加入空闲的buflist
	this->m_FreeBufferListLock.Lock();
	if(/*m_iMaxNumberOfFreeBuffer==0  ||*/m_FreeBufferList.GetCount()<m_iMaxNumberOfFreeBuffer)
	{//正常加入
		pIOBuf->SetPosition(NULL);
		pos=m_FreeBufferList.AddHead((void *)pIOBuf);
		
		
	}
	else
	{
		delete pIOBuf; 
		pIOBuf=NULL;
	}
	
	this->m_FreeBufferListLock.Unlock();
//	pIOBuf=NULL;
	return  TRUE;

}

//得到要处理的数据
CIOCPBuff * CNetEquipment::AllocateUseBuffer()
{

	CIOCPBuff  *pIOBuf=NULL;
	m_InUseBufferListLock.Lock();
//	int  icount=this->m_InUseBufferList.GetCount(); //得到缓冲区的个数;

#ifdef   _DEBUG
	
	//TRACE("1  AllocateUseBuffer  m_InUseBufferList  count  %d\n",m_InUseBufferList.GetCount());
#endif
    if(!m_InUseBufferList.IsEmpty())
	{
		pIOBuf=(CIOCPBuff *)m_InUseBufferList.RemoveHead();
	}
#ifdef   _DEBUG
	
	//TRACE("2  AllocateUseBuffer  m_InUseBufferList  count  %d\n",m_InUseBufferList.GetCount());
#endif
	m_InUseBufferListLock.Unlock();
	if(pIOBuf==NULL)
	{//没有分配到新的iocpbuf；

		return  NULL;
	}
	if(pIOBuf)
	{//得到了新的iocpbuf

		return  pIOBuf;
	}
	return  NULL;

}

//得到要处理的个数;
int   CNetEquipment::AllocateUseBufferNumber()
{
	
	m_InUseBufferListLock.Lock();
	int  icount=this->m_InUseBufferList.GetCount(); //得到缓冲区的个数;	
	m_InUseBufferListLock.Unlock();
	return  icount;

	
}
//得到整体的要处理的数据
//得到要处理的数据
void   CNetEquipment::AllocateUseBuffer(CObArray  &asUserBuffer)
{
	
	asUserBuffer.RemoveAll();
	asUserBuffer.SetSize(0);
	m_InUseBufferListLock.Lock();
	int  icount=this->m_InUseBufferList.GetCount(); //得到缓冲区的个数;	

	for(int  iLoop=0;iLoop<icount;iLoop++)
	{
		CIOCPBuff  *pIOBuf=NULL;		
		pIOBuf=(CIOCPBuff *)m_InUseBufferList.RemoveHead();
		if(pIOBuf)
		{
			asUserBuffer.Add((CObject  *)pIOBuf);
				
		}
		
	}
	m_InUseBufferListLock.Unlock();

	return ;
	
}



























BOOL CNetEquipment::End()
{

	this->m_beRun=FALSE;
	if(m_Tcpfd_HQGPS!=INVALID_SOCKET)
	{
		ReleaseSocket(m_Tcpfd_HQGPS);
	}
	if(m_Tcpfd_TQGPS!=INVALID_SOCKET)
	{
		ReleaseSocket(m_Tcpfd_TQGPS);
	}
	if(m_Udpfd_MTJJA!=INVALID_SOCKET)
	{
		ReleaseSocket(m_Udpfd_MTJJA);
	}
	if(m_Udpfd_MTGPS!=INVALID_SOCKET)
	{
		ReleaseSocket(m_Udpfd_MTGPS);
	}
    return  TRUE;
}

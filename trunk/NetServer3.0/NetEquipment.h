// NetEquipment.h: interface for the CNetEquipment class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETEQUIPMENT_H__419AF469_CFAF_459E_895D_499BB742BF2F__INCLUDED_)
#define AFX_NETEQUIPMENT_H__419AF469_CFAF_459E_895D_499BB742BF2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "Winsock2.h"

#include "SystemInfo.h"
#include "IOCPBuff.h"
#include <afxtempl.h>
#include <afxmt.h>
#include "EquipmentManager.h"


enum  OPERTYPE{
	    IOInitialize, // The client just connected
		IORead, // Read from the client. 
		IOReadCompleted, // Read completed
		IOWrite, // Write to the Client
		IOWriteCompleted, // Write Completed.
		IOZeroByteRead, // Read zero Byte from client (dummy for avoiding The System Blocking error) 
		IOZeroReadCompleted, // Read Zero Byte  completed. (se IOZeroByteRead)
		IOTransmitFileCompleted, //TransmitFileCompleted.
		IOPostedPackage, // Used to post Packages into IOCP port. 


};


class CIOCPBuff;
struct ClientContext;

typedef CMap<unsigned int, unsigned int&, ClientContext*, ClientContext* > ContextMap;
typedef CMap<unsigned int, unsigned int&, CIOCPBuff *, CIOCPBuff * > BufferMap;


/*定义一个设备连接的相关信息*/
struct ClientContext
{
	int  fd;   //the  current  fd;
	int  keyid;
	CIOCPBuff  *pIOCPBuff;  //接收缓冲区;
	BufferMap	m_ReadBufferMap; 
	BufferMap	m_SendBufferMap; 
	int        m_CurReadoderNune;  //当前可用序列的个数;
	int        m_CurSendooderNune;  //当前可用序列的个数;
	CMutex     m_ContextLock;
	int		   m_nNumberOfPendlingIO;
	int         m_EquipmentType;
//	CMutex     m_ContextLock;
//	int        m_EquipmentType;  //保存这个连接的设备的类型；
	//	CString    szkey;            //车机
	
};


class CNetEquipment  
{
public:
	void  AllocateUseBuffer(CObArray  &asUserBuffer);
	int   AllocateUseBufferNumber();
	int ExitIOLoop(ClientContext *pContext);
	BOOL End();
	CIOCPBuff * AllocateUseBuffer();
	BOOL ReleaseUseBuffer(CIOCPBuff  *pIOBuf);
	BOOL AddToUseBuffer(CIOCPBuff *pIOBuf);
	BOOL DisBuffer(CIOCPBuff  *pIOBuf);
	BOOL SocketBeInUdpfd(int  fd);
	void OnReadCompleted(ClientContext *pContext, DWORD dwIoSize,CIOCPBuff *pOverlapBuff);
	void OnRead(ClientContext *pContext,CIOCPBuff *pOverlapBuff);
	void OnZeroByteReadCompleted(ClientContext *pContext, DWORD dwIoSize,CIOCPBuff *pOverlapBuff);
	void OnZeroByteRead(ClientContext *pContext,CIOCPBuff *pOverlapBuff);
	BOOL ARead(ClientContext *pContext,CIOCPBuff*pOverlapBuff=NULL);
	BOOL AZeroByteRead(ClientContext *pContext, CIOCPBuff*pOverlapBuff);
	void NotifyNewConnection(ClientContext *pcontext);//处理一个新的连接；
	void OnInitialize(ClientContext *pContext, DWORD dwIoSize,CIOCPBuff *pOverlapBuff);
	BOOL ReleaseBuffer(CIOCPBuff  *pIOBuf);
	void DoNetMsg(CIOCPBuff *pOverlapBuff, ClientContext *pContext, DWORD dwSize);  //处理IOCP的消息的;这个是核心函数;
	void ReleaseClient(int fd);  //释放当前fd的所有信息;
	
	BOOL SetListenPro();
	BOOL  SetWorkPro();
	BOOL Start();
	CNetEquipment();
	virtual ~CNetEquipment();
	int  m_Tcpfd_HQGPS;
	int  m_Tcpfd_TQGPS;
	int   m_Udpfd_MTJJA;
	int   m_Udpfd_MTGPS;
	int  m_Tcpfd_GTQGPS; //20070606
	BOOL  m_beRun;
	HANDLE  m_IOCPHANDLE;
	BOOL AssociateIOCP(int  socket, HANDLE hCompletionPort, DWORD dwCompletionKey);
	BOOL CreateNewEqNode(int  fd,int iEquipmentType);  //创建一个新的设备网络结点;
	
   	ClientContext * AllocateContext();  //分派新的ClientContext
    CEquipmentManager    m_EquipmentManager;
	CIOCPBuff * AllocateBuffer(int nType);  //分配缓冲区
	
private:	
	void EnterIOLoop(ClientContext *pContext);
	BOOL ReleaseClientContext(ClientContext *pContext);
	void DisconnectClient(ClientContext  *pContext);
	BOOL AddClientContext(ClientContext *pContext);
	BOOL Run();
	//连接池
	CMutex  m_ContextMapLock;     //the  lock  of   ContextMap;
	//管理当前的所有连接的信息;
	ContextMap  m_ContextMap;      //the  map  off context 
	CMutex      m_FreeContextListLock;   //空闲的contextlist的锁
	CPtrList    m_FreeContextList;       //空闲的contextlist
	// 用于回收内存的; 
	CMutex       m_FreeBufferListLock;
	CPtrList     m_FreeBufferList;

	CMutex       m_BufferListLock;
	CPtrList     m_BufferList;    //正在使用的;

   //正在等待处理的接收到的数据列表;
	CMutex       m_InUseBufferListLock;
	CPtrList     m_InUseBufferList;

	int          m_CurAcctiveConntionNum;
	int           m_iMaxNumberOfFreeBuffer;// 控制空闲内存池的大小的；
	int          m_iNumberOfPendlingReads;  //需要进行多少次读操作;默认为一次
private:
#ifdef _DEBUG
	static   int  m_NewContextBufCount;
	static   int  m_NewIOCPBufCount;
	static   int  m_UserBufCount;
#endif
	




	
};

#endif // !defined(AFX_NETEQUIPMENT_H__419AF469_CFAF_459E_895D_499BB742BF2F__INCLUDED_)

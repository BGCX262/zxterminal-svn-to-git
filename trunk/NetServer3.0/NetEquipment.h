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


/*����һ���豸���ӵ������Ϣ*/
struct ClientContext
{
	int  fd;   //the  current  fd;
	int  keyid;
	CIOCPBuff  *pIOCPBuff;  //���ջ�����;
	BufferMap	m_ReadBufferMap; 
	BufferMap	m_SendBufferMap; 
	int        m_CurReadoderNune;  //��ǰ�������еĸ���;
	int        m_CurSendooderNune;  //��ǰ�������еĸ���;
	CMutex     m_ContextLock;
	int		   m_nNumberOfPendlingIO;
	int         m_EquipmentType;
//	CMutex     m_ContextLock;
//	int        m_EquipmentType;  //����������ӵ��豸�����ͣ�
	//	CString    szkey;            //����
	
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
	void NotifyNewConnection(ClientContext *pcontext);//����һ���µ����ӣ�
	void OnInitialize(ClientContext *pContext, DWORD dwIoSize,CIOCPBuff *pOverlapBuff);
	BOOL ReleaseBuffer(CIOCPBuff  *pIOBuf);
	void DoNetMsg(CIOCPBuff *pOverlapBuff, ClientContext *pContext, DWORD dwSize);  //����IOCP����Ϣ��;����Ǻ��ĺ���;
	void ReleaseClient(int fd);  //�ͷŵ�ǰfd��������Ϣ;
	
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
	BOOL CreateNewEqNode(int  fd,int iEquipmentType);  //����һ���µ��豸������;
	
   	ClientContext * AllocateContext();  //�����µ�ClientContext
    CEquipmentManager    m_EquipmentManager;
	CIOCPBuff * AllocateBuffer(int nType);  //���仺����
	
private:	
	void EnterIOLoop(ClientContext *pContext);
	BOOL ReleaseClientContext(ClientContext *pContext);
	void DisconnectClient(ClientContext  *pContext);
	BOOL AddClientContext(ClientContext *pContext);
	BOOL Run();
	//���ӳ�
	CMutex  m_ContextMapLock;     //the  lock  of   ContextMap;
	//����ǰ���������ӵ���Ϣ;
	ContextMap  m_ContextMap;      //the  map  off context 
	CMutex      m_FreeContextListLock;   //���е�contextlist����
	CPtrList    m_FreeContextList;       //���е�contextlist
	// ���ڻ����ڴ��; 
	CMutex       m_FreeBufferListLock;
	CPtrList     m_FreeBufferList;

	CMutex       m_BufferListLock;
	CPtrList     m_BufferList;    //����ʹ�õ�;

   //���ڵȴ�����Ľ��յ��������б�;
	CMutex       m_InUseBufferListLock;
	CPtrList     m_InUseBufferList;

	int          m_CurAcctiveConntionNum;
	int           m_iMaxNumberOfFreeBuffer;// ���ƿ����ڴ�صĴ�С�ģ�
	int          m_iNumberOfPendlingReads;  //��Ҫ���ж��ٴζ�����;Ĭ��Ϊһ��
private:
#ifdef _DEBUG
	static   int  m_NewContextBufCount;
	static   int  m_NewIOCPBufCount;
	static   int  m_UserBufCount;
#endif
	




	
};

#endif // !defined(AFX_NETEQUIPMENT_H__419AF469_CFAF_459E_895D_499BB742BF2F__INCLUDED_)

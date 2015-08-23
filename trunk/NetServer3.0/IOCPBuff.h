// IOCPBuff.h: interface for the CIOCPBuff class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IOCPBUFF_H__E2C4388C_F7A0_498A_94F3_2014001F4278__INCLUDED_)
#define AFX_IOCPBUFF_H__E2C4388C_F7A0_498A_94F3_2014001F4278__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//主要用来存储接收到的数据
#include "Winsock2.h"

#define   MAXIOCPBUFSIZE 512   //用来接收数据的缓冲区的最大空间的值;

class CIOCPBuff  
{
public:
	WSABUF * GetWSABuffer();
	void SetupZeroByteRead();
	int Getoperation();
	POSITION GetPosition();
	void SetPosition(POSITION pos);
	void SetSequenceNumber(int  unm);
	void SetOpertion(int  oper);
	void EmptyUsed();
	void SetupRead();  //准备好读写的缓冲区;
	void Init();  //initilze  the   CIOCPBuff
	CIOCPBuff();
	virtual ~CIOCPBuff();
	OVERLAPPED			m_ol;
	int   fd;//记录socket的id;
//	char  m_pIP; //指向来源的ip；
//	short m_iPort;//指向端口 主要是udp的时候使用的；
	int   m_iEuqType;//记录来源的设备型号;
	struct   sockaddr_in  addr;  //记录接受的addr的信息主要是udp是使用;
	char    szKey[15] ;//记录设备的标识
	int     iMsgLen;
	char    buf[MAXIOCPBUFSIZE];
	WSABUF  m_wsabuf;
	
private:
	
	int     m_UsedUnnber;  // 当前有多少字节被使用了；
	int	  	m_Operation;  //当前的错做类型;
	int      m_iSequenceNumber ;//???
	POSITION  m_pPos;  //在buflist中的位置信息
};

#endif // !defined(AFX_IOCPBUFF_H__E2C4388C_F7A0_498A_94F3_2014001F4278__INCLUDED_)

// IOCPBuff.h: interface for the CIOCPBuff class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IOCPBUFF_H__E2C4388C_F7A0_498A_94F3_2014001F4278__INCLUDED_)
#define AFX_IOCPBUFF_H__E2C4388C_F7A0_498A_94F3_2014001F4278__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//��Ҫ�����洢���յ�������
#include "Winsock2.h"

#define   MAXIOCPBUFSIZE 512   //�����������ݵĻ����������ռ��ֵ;

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
	void SetupRead();  //׼���ö�д�Ļ�����;
	void Init();  //initilze  the   CIOCPBuff
	CIOCPBuff();
	virtual ~CIOCPBuff();
	OVERLAPPED			m_ol;
	int   fd;//��¼socket��id;
//	char  m_pIP; //ָ����Դ��ip��
//	short m_iPort;//ָ��˿� ��Ҫ��udp��ʱ��ʹ�õģ�
	int   m_iEuqType;//��¼��Դ���豸�ͺ�;
	struct   sockaddr_in  addr;  //��¼���ܵ�addr����Ϣ��Ҫ��udp��ʹ��;
	char    szKey[15] ;//��¼�豸�ı�ʶ
	int     iMsgLen;
	char    buf[MAXIOCPBUFSIZE];
	WSABUF  m_wsabuf;
	
private:
	
	int     m_UsedUnnber;  // ��ǰ�ж����ֽڱ�ʹ���ˣ�
	int	  	m_Operation;  //��ǰ�Ĵ�������;
	int      m_iSequenceNumber ;//???
	POSITION  m_pPos;  //��buflist�е�λ����Ϣ
};

#endif // !defined(AFX_IOCPBUFF_H__E2C4388C_F7A0_498A_94F3_2014001F4278__INCLUDED_)

#ifndef _ITCP_INCLUDE_20110124_
#define _ITCP_INCLUDE_20110124_

#include "Common.h"
#include "ITigerNet.h"

#ifdef __LOG__
#include "LogWriter.h" 
#endif

#include "Connection.h"

class CTigerClient : public ITCPClient
{
public:
	CTigerClient(ITCPClientNotify* pTCPClientNotifyObj);
	virtual ~CTigerClient(void);

public:
	//����MDFЭ���TCP������
	virtual void ConnectMDFServer( const char *szIP, unsigned short usPort, bool bAuth=false, unsigned int nTimeout = 5 );

	//�������ݸ�MDFЭ��ķ���������ConnectMDFServer���ʹ�ã�
	virtual bool SendDataToMDFServer( const unsigned char * pucData, unsigned int unDataSize );

	//���ӷ�MDFЭ���TCP���������������еķ���������ƣ�
	virtual void ConnectOtherServer( const char *szIP, unsigned short usPort, bool bAuth=false, unsigned int nTimeout = 5 );

	//�������ݸ���MDFЭ��ķ���������ConnectOtherServer���ʹ�ã��������еķ���������ƣ�
	virtual bool SendDataToOtherServer( const unsigned char * pucData, unsigned int unDataSize );

	//��������Ͽ���������
	virtual void CloseSocket( bool bExitFlag=false );

public:
	//֪ͨ����
	ITCPClientNotify* m_pTCPClientNotifyObj;

	connection_pt m_pConn;

	boost::mutex m_cond_exit_mutex;

	boost::condition_variable m_CondExitNotify;
};

#endif

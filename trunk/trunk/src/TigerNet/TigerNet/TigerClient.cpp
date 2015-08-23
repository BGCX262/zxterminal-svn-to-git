// CommonClient.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "TigerClient.h"

#ifdef __LOG__
//��־
extern CLogWriter m_LogWriter;
#endif

//==================================================================
//��������CreateMDFTCPClientInstance  
//���ߣ� yin hui   
//���ڣ� 20110128  
//���ܣ�  ����CCommonClientʵ���Ľӿں���  
//���������  ITCPClientNotify* pTCPClientNotifyObj
//����ֵ��  CCommonClientʵ��
//�޸ļ�¼��ȡ���������boost::asio::io_service& io
//==================================================================

ITCPClient* CreateMDFTCPClientInstance(  ITCPClientNotify* pTCPClientNotifyObj )
{
	if ( pTCPClientNotifyObj==NULL )
		return NULL;

#ifdef __LOG__
	if (m_LogWriter.Initialize())
	{
		CLogWriter::Log(WN_LOG_LEVEL_INFO,"------------------------");
		CLogWriter::Log(WN_LOG_LEVEL_INFO,"[Initialize log succeed]");
	} 
#endif

	return new CTigerClient( pTCPClientNotifyObj );
}

//==================================================================
//�������� CCommonClient 
//���ߣ� yin hui   
//���ڣ� 20110128  
//���ܣ� CCommonClient���캯������ʼ����   
//���������ITCPClientNotify* pTCPClientNotifyObj
//����ֵ��  ��
//�޸ļ�¼��ȡ���������boost::asio::io_service& io
//==================================================================
CTigerClient::CTigerClient(ITCPClientNotify* pTCPClientNotifyObj)
:m_pTCPClientNotifyObj(pTCPClientNotifyObj)
{

}

 //==================================================================
 //��������~CCommonClient  
 //���ߣ� yin hui   
 //���ڣ� 20110128   
 //���ܣ� CCommonClient�����������ͷ���Դ���ر�socket   
 //���������
 //����ֵ��  
 //�޸ļ�¼��
 //==================================================================
CTigerClient::~CTigerClient(void)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::~CCommonClient] [IN]");
#endif

	if (m_pConn.get())
	{
		m_pConn.reset();
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::~CCommonClient] [OUT]");

	m_LogWriter.Uninitialize();
#endif
}



//==================================================================
//�������� ConnectMDFServer 
//���ߣ� yin hui   
//���ڣ� 20110128     
//���ܣ� ����ָ����TCP������  
//���������const char *szIP          IP��ַ 
//			unsigned short usPort     �˿�
//			bool bAuth                �Ƿ���Ҫ��֤
//����ֵ��  ��
//�޸ļ�¼��
//			20110128   �����첽��Ϣ����ѭ��m_pIO->run()�������ģ���˴���һ���߳�����   
//			20110218   ����keep_aliveѡ��
//==================================================================
void CTigerClient::ConnectMDFServer( const char *szIP, unsigned short usPort,bool bAuth, unsigned int nTimeout)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::ConnectMDFServer] [IN]");
#endif

	m_pConn = boost::shared_ptr<CConnection>(new CConnection(this));
	if (m_pConn.get())
	{
		m_pConn->ConnectMDFServer(szIP, usPort, bAuth, nTimeout);
	}
	

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::ConnectMDFServer] [OUT]\n");
#endif
}

//==================================================================
//�������� ConnectMDFServer 
//���ߣ� yin hui   
//���ڣ� 20110128     
//���ܣ� ���ӷ�MDFЭ���TCP������
//���������const char *szIP          IP��ַ 
//			unsigned short usPort     �˿�
//			bool bAuth                �Ƿ���Ҫ��֤
//����ֵ��  ��
//�޸ļ�¼��
//			20110128   �����첽��Ϣ����ѭ��m_pIO->run()�������ģ���˴���һ���߳�����   
//			20110218   ����keep_aliveѡ��
//==================================================================
void CTigerClient::ConnectOtherServer( const char *szIP, unsigned short usPort, bool bAuth, unsigned int nTimeout )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::ConnectOtherServer] [IN]");
#endif

	m_pConn = boost::shared_ptr<CConnection>(new CConnection(this));
	if (m_pConn.get())
	{
		m_pConn->ConnectOtherServer(szIP, usPort, bAuth, nTimeout);
	}
	
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::ConnectOtherServer] [OUT]\n");
#endif
}

//�������ݸ���MDFЭ��ķ�����
bool CTigerClient::SendDataToOtherServer( const unsigned char * pucData, unsigned int unDataSize )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::SendDataToOtherServer] [IN]");
#endif

	if (m_pConn.get())
	{
		m_pConn->SendDataToOtherServer(pucData, unDataSize);
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::SendDataToOtherServer] [OUT]\n");
#endif

	return true;
}

//==================================================================
//��������  SendDataToMDFServer
//���ߣ� yin hui   
//���ڣ� 20110128     
//���ܣ� �������ݸ�������   
//���������unsigned char * pData,				���ݻ���
//			unsigned int ulDataSize	���ݳ���
//����ֵ��  TRUE/FALSE
//�޸ļ�¼��20110215 �޸�CPacket���������������Ҹ�Ϊshared_ptr����֤ÿ����Դ�ͷţ������ڴ�й¶
//          20110506 �ڻص�����send_data_handler������packet_pt pPacket��������������첽���ͻ�û�����
//                   ���ͷŻ��壬���³��֡�ϵͳ��⵽��һ�������г���ʹ��ָ�����ʱ����Чָ���ַ��������
//==================================================================
bool CTigerClient::SendDataToMDFServer( const unsigned char * pucData, unsigned int unDataSize )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::SendDataToMDFServer] [IN]");
#endif

	if (m_pConn.get())
	{
		m_pConn->SendDataToMDFServer(pucData, unDataSize);
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::SendDataToMDFServer] [OUT]\n");
#endif

	return true;
}


//==================================================================
//�������� CloseSocket 
//���ߣ� yin hui   
//���ڣ� 20110128     
//���ܣ� ��������Ͽ��������ӣ��˳����������ݽ��գ��ر�socket  
//�����������
//����ֵ��  ��
//�޸ļ�¼����
//==================================================================
void CTigerClient::CloseSocket( bool bExitFlag )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::CloseSocket] [IN]");
#endif

	if (m_pConn.get())
	{
		m_pConn->CloseSocket(bExitFlag);
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::CloseSocket] [OUT]\n");
#endif
}


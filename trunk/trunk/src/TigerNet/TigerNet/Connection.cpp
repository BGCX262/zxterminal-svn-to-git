// CommonClient.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "Connection.h"

#include "TigerClient.h"

//==================================================================
//�������� CConnection 
//���ߣ� yin hui   
//���ڣ� 20110128  
//���ܣ� CConnection���캯������ʼ����   
//���������ITCPClientNotify* pTCPClientNotifyObj
//����ֵ��  ��
//�޸ļ�¼��ȡ���������boost::asio::io_service& io
//==================================================================
CConnection::CConnection(ITCPClient* pClient): m_pClient(pClient)
{
	m_tLastRecv = (long)time(0);

	m_pIO.reset();
	m_strand = NULL;

	//m_ReadTimer.reset();
	m_ConnectTimer.reset();
	m_HBTimer.reset();
	m_HBCheckTimer.reset();

	m_bConnect = false;
	m_bAppExit = false;

	m_nThreadPoolSize = 1; //sysconf(_SC_NPROCESSORS_ONLN)

	m_nErrorCode = 0;

	m_nMaxBodyLength = MAX_BODY_LENGTH;

	//pCRCBuffer = new unsigned char[MAX_BODY_LENGTH];

	m_pchHead = new unsigned char[HEADER_LENGTH];
	m_pchBody = new unsigned char[MAX_BODY_LENGTH];

}

 //==================================================================
 //��������~CConnection  
 //���ߣ� yin hui   
 //���ڣ� 20110128   
 //���ܣ� CConnection�����������ͷ���Դ���ر�socket   
 //���������
 //����ֵ��  
 //�޸ļ�¼��
 //==================================================================
CConnection::~CConnection(void)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::~CConnection] [IN]");
#endif

	//������������io_service��صĶ��󣬲����������io_service
	if (m_pIO)
	{
		if (m_ConnectTimer)
		{
			m_ConnectTimer->cancel();
			m_ConnectTimer.reset();
		}

		if (m_HBTimer)
		{
			m_HBTimer->cancel();
			m_HBTimer.reset();
		}

		if (m_HBCheckTimer)
		{
			m_HBCheckTimer->cancel();
			m_HBCheckTimer.reset();
		}

		if (m_socket)
		{
			m_socket->close();
			m_socket.reset();
		}

		if (m_strand)
		{
			delete m_strand;
			m_strand = NULL;
		}

		m_pIO.reset();
	}

	//if (pCRCBuffer)
	//{
	//	delete[] pCRCBuffer;
	//	pCRCBuffer = NULL;
	//}

	if (m_pchHead)
	{
		delete[] m_pchHead;
		m_pchHead = NULL;
	}

	if (m_pchBody)
	{
		delete[] m_pchBody;
		m_pchBody = NULL;
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::~CConnection] [IN]");
#endif
}

//==================================================================
//�������� Run 
//���ߣ� yin hui   
//���ڣ� 20110128    
//���ܣ� �����첽��Ϣ����ѭ��������˳������޸ı�־���Ա��˳������̺߳����ݽ���   
//�����������
//����ֵ�� �� 
//�޸ļ�¼��20110221 Run()���߳����޸�m_socket��disconnect_handler���޸ģ������½������ݳ���ȡ������disconnect_handler
//==================================================================
void CConnection::Run()
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::Run] [IN]");
#endif

	try
	{
		if (m_pIO)
		{
			m_pIO->run();
		}
	}
	catch (std::exception& e)
	{
#ifdef __LOG__
		char szLogMsg[1024];
		sprintf(szLogMsg, "[CCommonServer::Run][Error = %s]", e.what());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLogMsg);
#endif

		disconnect_handler(m_socket, E_WN_CLIENT_IOSERVICE_EXCEPTION);
		m_nErrorCode = E_WN_CLIENT_IOSERVICE_EXCEPTION;
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::Run] [OUT]\n");
#endif
}

//20105016 �Ѵ���֪ͨ�ŵ����е�Run�˳����ִ��
void CConnection::BeginRunThread()
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::BeginRunThread] [IN]");
#endif

	for (int i = 0; i < m_nThreadPoolSize; i ++)
	{
		m_ThreadGroup.create_thread(boost::bind(&CConnection::Run, this));
	}

	m_ThreadGroup.join_all();

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_INFO,"[CConnection::BeginRunThread] [End of io_service::run()]");

	char szLog[1024];
	sprintf(szLog, "[CConnection::BeginRunThread] [m_nErrorCode = %d]", m_nErrorCode);
	CLogWriter::Log(WN_LOG_LEVEL_INFO,szLog);

#endif

	CTigerClient* pCommonClient = NULL;
	if (m_pClient)
	{
		pCommonClient = static_cast<CTigerClient*>(m_pClient);
	}

	switch (m_nErrorCode)
	{
	case E_WN_CLIENT_CONNECT_FAIL:
	case E_WN_CLIENT_SEND_AUTH_INFO_FAIL:
	case E_WN_CLIENT_RECV_AUTH_INFO_FAIL:
		if ( !m_bAppExit && pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnConnectServer(m_nErrorCode);
		}
		break;

	case E_WN_CLIENT_SEND_HB_FAIL:
	case E_WN_CLIENT_RECV_HEAD_FAIL:
	case E_WN_CLIENT_RECV_BODY_FAIL:
	case E_WN_CLIENT_SEND_DATA_FAIL:
	case E_WN_CLIENT_SEND_LOGOUT_FAIL:
	case E_WN_CLIENT_CONNECT_TIMEOUT:
	case E_WN_CLIENT_RECV_TIMEOUT:
	case E_WN_CLIENT_RECV_NOMDF_FAIL:
	case E_WN_CLIENT_RECV_NO_HB:
	case E_WN_CLIENT_CLOSE_SOCKET:
	case E_WN_CLIENT_APP_EXIT:
	case E_WN_CLIENT_UNKNOWN:
	case E_WN_CLIENT_IOSERVICE_EXCEPTION:
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDisconnect(m_nErrorCode);
		}

		break;
	
	default :
		break;
	}



#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::BeginRunThread] [OUT]");
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
void CConnection::ConnectMDFServer( const char *szIP, unsigned short usPort,bool bAuth, int nTimeout)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::ConnectMDFServer] [IN]");
#endif

	m_bConnect = false;

	//��֤IP��ַ
	//��֤�˿�

	m_pIO = boost::shared_ptr<boost::asio::io_service>(new boost::asio::io_service());
	m_strand = new boost::asio::strand(*m_pIO);

	//m_ReadTimer =  make_shared<deadline_timer>(ref(*m_pIO));
	m_ConnectTimer	= boost::make_shared<boost::asio::deadline_timer>(boost::ref(*m_pIO));
	m_HBTimer		= boost::make_shared<boost::asio::deadline_timer>(boost::ref(*m_pIO));
	m_HBCheckTimer	= boost::make_shared<boost::asio::deadline_timer>(boost::ref(*m_pIO));
	m_socket		= sock_pt(new boost::asio::ip::tcp::socket(*m_pIO));

	//�첽���ӷ�����

	//boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(szIP),usPort);

	//boost::asio::socket_base::reuse_address opt(1);
	//boost::system::error_code ec;
	//m_socket->set_option(opt,ec);
	//if (ec > 0 )
	//{
	//	std::string strError = ec.message();
	//}

	try
	{
		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(szIP),usPort);

		if (m_socket)
		{
			m_socket->async_connect(ep, m_strand->wrap(bind(&CConnection::connect_handler, shared_from_this(), m_socket, boost::asio::placeholders::error, szIP, usPort, bAuth)));
		}
	}
	catch (std::exception& e)
	{
		std::string str = e.what();

		disconnect_handler(m_socket, E_WN_CLIENT_CONNECT_FAIL);
		m_nErrorCode = E_WN_CLIENT_CONNECT_FAIL;

#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::ConnectMDFServer] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);

		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::ConnectMDFServer] [OUT]\n");
#endif

		CTigerClient* pCommonClient = NULL;
		if (m_pClient)
		{
			pCommonClient = static_cast<CTigerClient*>(m_pClient);

			if ( pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
			{
				pCommonClient->m_pTCPClientNotifyObj->OnConnectServer(m_nErrorCode);
			}
		}
	
		return;
	}

	//�������ӳ�ʱ��ʱ��
	m_ConnectTimer->expires_from_now(boost::posix_time::seconds(nTimeout));
	m_ConnectTimer->async_wait(m_strand->wrap(bind(&CConnection::connect_timer_handler, shared_from_this(), m_socket, boost::asio::placeholders::error)));

	//�����첽��Ϣ����ѭ���߳�
	boost::thread t(&CConnection::BeginRunThread, shared_from_this());

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::ConnectMDFServer] [OUT]\n");
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
void CConnection::ConnectOtherServer( const char *szIP, unsigned short usPort, bool bAuth, int nTimeout )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::ConnectOtherServer] [IN]");
#endif

	m_bConnect = false;

	m_pIO = boost::shared_ptr<boost::asio::io_service>(new boost::asio::io_service());
	m_strand = new boost::asio::strand(*m_pIO);

	//m_ReadTimer =  make_shared<deadline_timer>(ref(*m_pIO));
	m_ConnectTimer	= boost::make_shared<boost::asio::deadline_timer>(boost::ref(*m_pIO));
	m_HBTimer		= boost::make_shared<boost::asio::deadline_timer>(boost::ref(*m_pIO));
	m_HBCheckTimer	= boost::make_shared<boost::asio::deadline_timer>(boost::ref(*m_pIO));
	m_socket		= sock_pt(new boost::asio::ip::tcp::socket(*m_pIO));

	//�첽���ӷ�����

	//boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(szIP),usPort);

	try
	{
		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(szIP),usPort);

		if (m_socket)
		{
			m_socket->async_connect(ep, m_strand->wrap(boost::bind(&CConnection::connect_NOT_MDF_handler, shared_from_this(), m_socket, boost::asio::placeholders::error,szIP,usPort,bAuth)));
		}
	}
	catch (std::exception& e)
	{
		std::string str = e.what();

		disconnect_handler(m_socket, E_WN_CLIENT_CONNECT_FAIL);
		m_nErrorCode = E_WN_CLIENT_CONNECT_FAIL;

#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::ConnectOtherServer] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);

		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::ConnectOtherServer] [OUT]\n");
#endif

		CTigerClient* pCommonClient = NULL;
		if (m_pClient)
		{
			pCommonClient = static_cast<CTigerClient*>(m_pClient);

			if ( pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
			{
				pCommonClient->m_pTCPClientNotifyObj->OnConnectServer(m_nErrorCode);
			}
		}

		return;
	}

	//�������ӳ�ʱ��ʱ��
	m_ConnectTimer->expires_from_now(boost::posix_time::seconds(nTimeout));
	m_ConnectTimer->async_wait(m_strand->wrap(boost::bind(&CConnection::connect_timer_handler, shared_from_this(), m_socket, boost::asio::placeholders::error)));

	//�����첽��Ϣ����ѭ���߳�
	//boost::thread t(&CConnection::Run,this);
	boost::thread t(&CConnection::BeginRunThread, shared_from_this());

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::ConnectOtherServer] [OUT]\n");
#endif
}

//�������ݸ���MDFЭ��ķ�����
bool CConnection::SendDataToOtherServer( const unsigned char * pucData, unsigned int unDataSize )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::SendDataToOtherServer] [IN]");
#endif

	if ( !m_socket )
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::SendDataToOtherServer] [OUT]");
#endif
		return false;
	}

	nonmdfpacket_pt pPacket(new CNonMDFPacket(pucData,unDataSize));

	m_pIO->post(m_strand->wrap(boost::bind(&CConnection::OnSendOtherData, shared_from_this(), m_socket, pPacket)));

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::SendDataToOtherServer] [OUT]\n");
#endif

	return true;
}

void CConnection::send_NOT_MDF_data_handler(sock_pt sock,const boost::system::error_code& ec, std::size_t bytes_transferred, unsigned int ulDataSize )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::send_NOT_MDF_data_handler] [IN]");
#endif

	if( ec || !m_socket )
	{
		disconnect_handler(sock,E_WN_CLIENT_SEND_DATA_FAIL);
		m_nErrorCode = E_WN_CLIENT_SEND_DATA_FAIL;
#ifdef __LOG__
		char szLog[1024];
		std::string str =ec.message();
		sprintf(szLog, "[CConnection::send_NOT_MDF_data_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

		return;
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::send_NOT_MDF_data_handler] [OUT]\n");
#endif
}

//==================================================================
//��������  connect_timer_handler
//���ߣ� yin hui   
//���ڣ� 20110128     
//���ܣ� ���ӳ�ʱ�ص�����   
//���������const system::error_code& ec ������
//����ֵ��  ��
//�޸ļ�¼����
//==================================================================
void CConnection::connect_timer_handler(sock_pt sock, const boost::system::error_code& ec)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::connect_timer_handler] [IN]");
#endif

	if( ec ) //timer�Ѿ�ȡ��
	{
		//std::cout << "timer is cancelled." << endl;
	}
	else  //timer�Ѿ���ʱ
	{
		disconnect_handler(sock,E_WN_CLIENT_CONNECT_TIMEOUT);
		m_nErrorCode = E_WN_CLIENT_CONNECT_TIMEOUT;
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::connect_timer_handler] [OUT]\n");
#endif
}

//==================================================================
//�������� connect_handler 
//���ߣ� yin hui   
//���ڣ� 20110128     
//���ܣ�  �첽���ӻص�����  
//���������const system::error_code& ec,	������
//			sock_pt sock,					�������ӵ�socket
//			const char *szIP,				IP��ַ
//			unsigned short usPort,			�˿�
//			bool bAuth						�Ƿ���Ҫ��֤
//����ֵ��  ��
//�޸ļ�¼��20100314 ����disconnect_handler�ر�socket�������һ�����Ӳ��ϵڶ������ӳ����������⣨io_serviceû�˳���
//==================================================================
void CConnection::connect_handler(sock_pt sock, const boost::system::error_code& ec, const char *szIP, unsigned short usPort, bool bAuth)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::connect_handler] [IN]");
#endif

	//ȡ��timer
	if (m_ConnectTimer)
	{
		m_ConnectTimer->cancel();
	}

	CTigerClient* pCommonClient = NULL;
	if (m_pClient)
	{
		pCommonClient = static_cast<CTigerClient*>(m_pClient);
	}

	if ( ec || !m_socket || !m_strand || !sock )
	{
		disconnect_handler(sock,E_WN_CLIENT_CONNECT_FAIL);
		m_nErrorCode = E_WN_CLIENT_CONNECT_FAIL;
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CConnection::connect_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);

		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::connect_handler] [OUT]\n");
#endif

		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			//pCommonClient->m_pTCPClientNotifyObj->OnConnectServer(m_nErrorCode);
		}
		return;
	}

	boost::asio::ip::tcp::socket::send_buffer_size nSendBufSize1(-1);
	boost::asio::ip::tcp::socket::receive_buffer_size nRecvBufSize1(-1); 

	boost::asio::ip::tcp::socket::send_buffer_size nSendBufSize(2 * 1024 * 1024);
	boost::asio::ip::tcp::socket::receive_buffer_size nRecvBufSize(2 * 1024 * 1024); 

	sock->get_option(nSendBufSize1);
	sock->get_option(nRecvBufSize1);
	//sock->set_option(nSendBufSize);
	//sock->set_option(nRecvBufSize);

	if (bAuth)
	{
	}
	else
	{
		m_bConnect = true;

		m_HBTimer->expires_from_now(boost::posix_time::seconds(HB_SEND_PERIOD));
		m_HBTimer->async_wait(m_strand->wrap(boost::bind(&CConnection::HB_timer_handler, shared_from_this(), sock, boost::asio::placeholders::error)));

		RecvData( sock );

		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnConnectServer(E_WN_CLIENT_CONNECT_OK);
		}
		
	}

	//�����������
	m_HBCheckTimer->expires_from_now(boost::posix_time::seconds(3 * HB_SEND_PERIOD));
	m_HBCheckTimer->async_wait(m_strand->wrap(boost::bind(&CConnection::check_hb_timer_handler, shared_from_this(), sock, boost::asio::placeholders::error)));

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::connect_handler] [OUT]\n");
#endif
}


//==================================================================
//��������  check_hb_timer_handler
//���ߣ� yin hui   
//���ڣ� 20110211      
//���ܣ�  ���������ʱ���Ļص�����
//���������const system::error_code& ec    ������
//����ֵ��  ��
//�޸ļ�¼����
//==================================================================
void CConnection::check_hb_timer_handler(sock_pt sock, const boost::system::error_code& ec)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::check_hb_timer_handler] [IN]");
#endif

	if (!m_bConnect || m_bAppExit)
	{
		return;
	}

	if ( !m_socket || !sock || !m_strand )
	{
		disconnect_handler(sock,E_WN_CLIENT_RECV_NO_HB);
		m_nErrorCode = E_WN_CLIENT_RECV_NO_HB;

		return;
	}

	//��ʱ����ȡ��
	if ( ec )
	{
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CConnection::check_hb_timer_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		return;
	}

	long lTime = (long)time(0);
	if (lTime - m_tLastRecv < 3 * HB_SEND_PERIOD)
	{
		m_HBCheckTimer->expires_from_now(boost::posix_time::seconds(3 * HB_SEND_PERIOD - (lTime - m_tLastRecv)));
		m_HBCheckTimer->async_wait(m_strand->wrap(boost::bind(&CConnection::check_hb_timer_handler, shared_from_this(), sock, boost::asio::placeholders::error)));
	}
	else //����45sû�н��յ�����
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CConnection::check_hb_timer_handler] [����45��û�н��յ�����]");
#endif

		disconnect_handler(sock,E_WN_CLIENT_RECV_NO_HB);
		m_nErrorCode = E_WN_CLIENT_RECV_NO_HB;
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::check_hb_timer_handler] [OUT]\n");
#endif
}


//==================================================================
//�������� connect_NOT_MDF_handler 
//���ߣ� yin hui   
//���ڣ� 20110128     
//���ܣ�  �첽���ӻص�����  
//���������const system::error_code& ec,	������
//			sock_pt sock,					�������ӵ�socket
//			const char *szIP,				IP��ַ
//			unsigned short usPort,			�˿�
//			bool bAuth						�Ƿ���Ҫ��֤
//����ֵ��  ��
//�޸ļ�¼��20100314 ����disconnect_handler�ر�socket�������һ�����Ӳ��ϵڶ������ӳ����������⣨io_serviceû�˳���
//==================================================================
void CConnection::connect_NOT_MDF_handler(sock_pt sock, const boost::system::error_code& ec, const char *szIP, unsigned short usPort,bool bAuth)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::connect_NOT_MDF_handler] [IN]");
#endif

	//ȡ��timer
	if (m_ConnectTimer)
	{
		m_ConnectTimer->cancel();
	}

	CTigerClient* pCommonClient = NULL;
	if (m_pClient)
	{
		pCommonClient = static_cast<CTigerClient*>(m_pClient);
	}

	if ( ec || !m_socket || !sock )
	{
		disconnect_handler(sock,E_WN_CLIENT_CONNECT_FAIL);
		m_nErrorCode = E_WN_CLIENT_CONNECT_FAIL;
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CConnection::connect_NOT_MDF_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			//pCommonClient->m_pTCPClientNotifyObj->OnConnectServer(m_nErrorCode);
		}

		return;
	}

	if (bAuth)
	{
	}
	else
	{
		m_bConnect = true;

#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::connect_NOT_MDF_handler] call RecvNOTMDFData]");
#endif
		RecvNOTMDFData( sock );

		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnConnectServer(E_WN_CLIENT_CONNECT_OK);
		}
		
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::connect_NOT_MDF_handler] [OUT]\n");
#endif
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
bool CConnection::SendDataToMDFServer( const unsigned char * pucData, unsigned int unDataSize )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::SendDataToMDFServer] [IN]");
#endif

	if ( !m_socket )
		return false;

	packet_pt pPacket(new CPacket(1,0,pucData,unDataSize));
	m_pIO->post(m_strand->wrap(boost::bind(&CConnection::OnSendMDFData, shared_from_this(), m_socket, pPacket)));

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::SendDataToMDFServer] [OUT]\n");
#endif

	return true;
}

//==================================================================
//��������  send_data_handler
//���ߣ� yin hui   
//���ڣ� 20110128    
//���ܣ��������ݵĻص�����
//���������onst system::error_code& ec ������
//����ֵ��  ��
//�޸ļ�¼����
//==================================================================
void CConnection::send_data_handler(sock_pt sock, const boost::system::error_code& ec, size_t bytes_transferred, packet_pt pPacket, unsigned int unDataLen)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::send_data_handler] [IN]");
#endif

	if (!m_bConnect || m_bAppExit)
	{
		return;
	}

	if( ec || !m_socket )
	{
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CConnection::send_data_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

		disconnect_handler(sock,E_WN_CLIENT_SEND_DATA_FAIL);
		m_nErrorCode = E_WN_CLIENT_SEND_DATA_FAIL;

		return;
	}

	if (bytes_transferred != unDataLen)
	{
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CTCPClient::send_data_handler] Ӧ����[%d] �ѷ���[%d]", unDataLen, bytes_transferred);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

		disconnect_handler(sock,E_WN_CLIENT_SEND_DATA_FAIL);
		m_nErrorCode = E_WN_CLIENT_SEND_DATA_FAIL;
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::send_data_handler] [OUT]\n");
#endif
}


//==================================================================
//��������  send_mdfdata_handler
//���ߣ� yin hui   
//���ڣ� 20110602    
//���ܣ�����MDF���ݵĻص�����
//���������onst system::error_code& ec ������
//����ֵ��  ��
//�޸ļ�¼����
//==================================================================
void CConnection::send_mdfdata_handler(sock_pt sock, const boost::system::error_code& ec, size_t bytes_transferred, packet_pt pPacket)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::send_mdfdata_handler] [IN]");
#endif

	if (!m_bConnect || m_bAppExit)
	{
		return;
	}

	if( ec || !m_socket || !sock )
	{
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CConnection::send_mdfdata_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

		disconnect_handler(sock,E_WN_CLIENT_SEND_DATA_FAIL);
		m_nErrorCode = E_WN_CLIENT_SEND_DATA_FAIL;

		return;
	}

	unsigned int unDataLen = pPacket->GetSize();
	if (bytes_transferred != unDataLen)
	{
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CConnection::send_mdfdata_handler] Ӧ����[%d] �ѷ���[%d]", unDataLen, bytes_transferred);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

		disconnect_handler(sock,E_WN_CLIENT_SEND_DATA_FAIL);
		m_nErrorCode = E_WN_CLIENT_SEND_DATA_FAIL;
	}
	else
	{
		//���������ʱ��
		m_tLastSend = (long)time(0);

		m_listSendData.pop_front();
		if ( !m_listSendData.empty() )
		{
			packet_pt pNextPacket = m_listSendData.front();
			if ( sock )
			{
				async_write(*sock, boost::asio::buffer(pNextPacket->GetBuffer(),pNextPacket->GetSize()), m_strand->wrap(boost::bind(&CConnection::send_mdfdata_handler, 
					shared_from_this(), sock, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, pNextPacket)));
			}
		}

	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::send_mdfdata_handler] [OUT]\n");
#endif

}

//==================================================================
//��������  send_nonmdfdata_handler
//���ߣ� yin hui   
//���ڣ� 20110128    
//���ܣ��������ݵĻص�����
//���������onst system::error_code& ec ������
//����ֵ��  ��
//�޸ļ�¼����
//==================================================================
void CConnection::send_nonmdfdata_handler(sock_pt sock, const boost::system::error_code& ec, size_t bytes_transferred, nonmdfpacket_pt pPacket)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::send_nonmdfdata_handler] [IN]");
#endif

	if (!m_bConnect || m_bAppExit)
	{
		return;
	}

	if( ec || !m_socket || !sock )
	{
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CConnection::send_nonmdfdata_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

		disconnect_handler(sock,E_WN_CLIENT_SEND_DATA_FAIL);
		m_nErrorCode = E_WN_CLIENT_SEND_DATA_FAIL;

		return;
	}

	unsigned int unDataLen = pPacket->GetSize();
	if (bytes_transferred != unDataLen)
	{
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CConnection::send_nonmdfdata_handler] Ӧ����[%d] �ѷ���[%d]", unDataLen, bytes_transferred);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

		disconnect_handler(sock,E_WN_CLIENT_SEND_DATA_FAIL);
		m_nErrorCode = E_WN_CLIENT_SEND_DATA_FAIL;
	}
	else
	{
		//���������ʱ��
		m_tLastSend = (long)time(0);

		pPacket.reset();

		m_listSendData_NonMdf.pop_front();
		if ( !m_listSendData_NonMdf.empty() )
		{
			nonmdfpacket_pt pNextPacket = m_listSendData_NonMdf.front();
			if ( sock )
			{
				async_write(*sock, boost::asio::buffer(pNextPacket->GetBuffer(),pNextPacket->GetSize()), m_strand->wrap(boost::bind(&CConnection::send_nonmdfdata_handler, 
					shared_from_this(), sock, boost::asio::placeholders::error,boost::asio:: placeholders::bytes_transferred, pNextPacket)));
			}
		}
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::send_nonmdfdata_handler] [OUT]\n");
#endif
}

//==================================================================
//�������� send_hb_handler 
//���ߣ� yin hui   
//���ڣ� 20110128    
//���ܣ� �����������Ļص�����   
//���������const system::error_code& ec ������
//����ֵ��  ��
//�޸ļ�¼����
//==================================================================
void CConnection::send_hb_handler(sock_pt sock, const boost::system::error_code& ec, size_t bytes_transferred, packet_pt pPacket, unsigned int unDataLen)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::send_hb_handler] [IN]");
#endif

	if (!m_bConnect || m_bAppExit)
	{
		return;
	}

	if ( ec || !m_socket || !sock )
	{
		disconnect_handler(sock,E_WN_CLIENT_SEND_HB_FAIL);
		m_nErrorCode = E_WN_CLIENT_SEND_HB_FAIL;
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CConnection::send_hb_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		return;
	} 

	if (bytes_transferred != unDataLen)
	{
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CTCPClient::send_hb_handler] Ӧ����[%d] �ѷ���[%d]", unDataLen, bytes_transferred);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

		disconnect_handler(sock,E_WN_CLIENT_SEND_HB_FAIL);
		m_nErrorCode = E_WN_CLIENT_SEND_HB_FAIL;
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::send_hb_handler] [OUT]\n");
#endif
}

//==================================================================
//�������� RecvData 
//���ߣ� yin hui   
//���ڣ� 20110128    
//���ܣ� ��������ѭ��   
//�����������
//����ֵ��  ��
//�޸ļ�¼����
//==================================================================
void CConnection::RecvData( sock_pt sock )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::RecvData] [IN]");
#endif

	if ( !m_socket || !sock || !m_strand )
		return;

	//�첽���հ�ͷ
	//boost::shared_ptr<unsigned char> btHead(new unsigned char[8], deleter);
	async_read(*sock, boost::asio::buffer(m_pchHead, 8),  m_strand->wrap(boost::bind(&CConnection::recv_head_handler,shared_from_this(),
		sock, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred/*, btHead*/)));

	//���ý��հ�ͷ��ʱ��ʱ��
	//if (m_ReadTimer)
	//{
	//	m_ReadTimer->expires_from_now(boost::posix_time::seconds(5));
	//	m_ReadTimer->async_wait(m_strand->wrap(bind(&CConnection::read_timer_handler, this,boost::asio::placeholders::error)));
	//}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::RecvData] [OUT]\n");
#endif
}

//==================================================================
//��������  read_timer_handler
//���ߣ� yin hui   
//���ڣ� 20110128    
//���ܣ� ���հ�ͷ��ʱ�ص�����   
//���������const system::error_code& ec ������
//����ֵ��  ��
//�޸ļ�¼��20110128  ��ʱ���ر�socket����������
//==================================================================
/*void CConnection::read_timer_handler( sock_pt sock, const system::error_code& ec)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::read_timer_handler] [IN]");
#endif

	if(ec) //timer�Ѿ�ȡ��
	{
		//std::cout << "timer is canceled." << endl;
	}
	else  //timer�Ѿ���ʱ
	{
		//std::cout << "time is over" << endl;
		//disconnect_handler(sock,E_WN_CLIENT_RECV_TIMEOUT);
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::read_timer_handler] [OUT]\n");
#endif

}
*/

void CConnection::OnSendMDFData(sock_pt sock, packet_pt pPacket)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::OnSendData] [IN]");
#endif

	if (!m_bConnect || m_bAppExit)
	{
		return;
	}

	if ( !m_socket || !sock )
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::OnSendMDFData] [OUT]");
#endif
		return;
	}

	if ( m_listSendData.empty() )
	{
		if ( sock )
		{
			async_write(*sock, boost::asio::buffer(pPacket->GetBuffer(),pPacket->GetSize()), m_strand->wrap(boost::bind(&CConnection::send_mdfdata_handler, shared_from_this(), 
				sock, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, pPacket)));

			m_listSendData.push_back( pPacket );
		}		
	}
	else
	{
		m_listSendData.push_back( pPacket );
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::OnSendMDFData] [OUT]");
#endif
}


void CConnection::OnSendOtherData(sock_pt sock, nonmdfpacket_pt pPacket)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::OnSendData] [IN]");
#endif

	if (!m_bConnect || m_bAppExit)
	{
		return;
	}

	if ( !m_socket || !sock )
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::OnSendData] [OUT]");
#endif
		return;
	}

	if ( m_listSendData_NonMdf.empty() )
	{
		async_write(*sock, boost::asio::buffer(pPacket->GetBuffer(),pPacket->GetSize()), m_strand->wrap(boost::bind(&CConnection::send_nonmdfdata_handler, shared_from_this(), 
			sock, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, pPacket)));

		m_listSendData_NonMdf.push_back( pPacket );
	}
	else
	{
		m_listSendData_NonMdf.push_back( pPacket );
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::OnSendData] [OUT]");
#endif
}


//==================================================================
//��������  HB_timer_handler
//���ߣ� yin hui   
//���ڣ� 20110128    
//���ܣ� ����������ʱ������   
//���������const system::error_code& ec ������
//����ֵ��  ��
//�޸ļ�¼��
//==================================================================
void CConnection::HB_timer_handler(sock_pt sock, const boost::system::error_code& ec)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::HB_timer_handler] [IN]");
#endif

	if (!m_bConnect || m_bAppExit)
	{
		return;
	}

	if ( ec || !m_socket || !m_strand || !sock )
	{
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CConnection::HB_timer_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		return;
	}

	packet_pt pHBPacket(new CPacket(2,0,NULL,1));
	m_pIO->post(m_strand->wrap(boost::bind(&CConnection::OnSendMDFData, shared_from_this(), sock, pHBPacket)));

	if (m_HBTimer)
	{
		m_HBTimer->expires_from_now(boost::posix_time::seconds(HB_SEND_PERIOD));
		m_HBTimer->async_wait(m_strand->wrap(boost::bind(&CConnection::HB_timer_handler, shared_from_this(), sock, boost::asio::placeholders::error)));
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::HB_timer_handler] [OUT]\n");
#endif
}

//==================================================================
//��������  recv_head_handler
//���ߣ� yin hui   
//���ڣ� 20110128    
//���ܣ� ���հ�ͷ�Ļص�����  
//���������const system::error_code& ec ������
//          shared_ptr<unsigned char> btHead      ���հ�ͷ�Ļ���
//����ֵ��  ��
//�޸ļ�¼����
//==================================================================
void CConnection::recv_head_handler( sock_pt sock, const boost::system::error_code& ec, size_t bytes_transferred/*, boost::shared_ptr<unsigned char> btHead*/)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_head_handler] [IN]");
#endif

	if (!m_bConnect || m_bAppExit)
	{
		return;
	}

	//ȡ�����ճ�ʱtimer
	//if (m_ReadTimer)
	//{
	//	m_ReadTimer->cancel();
	//}

	CTigerClient* pCommonClient = NULL;
	if (m_pClient)
	{
		pCommonClient = static_cast<CTigerClient*>(m_pClient);
	}

	if ( ec || !m_socket || !sock || !m_strand )
	{
		disconnect_handler(sock,E_WN_CLIENT_RECV_HEAD_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_HEAD_FAIL;
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CConnection::recv_head_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		return;
	} 

	//��֤��ͷ����
	if (bytes_transferred != 8)
	{
		//��������֪ͨ
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
		}

		disconnect_handler(sock,E_WN_CLIENT_RECV_HEAD_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_HEAD_FAIL;
#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_head_handler] [�����ֽ������� Ӧ����[%d] �Ѵ���[%d] = %d]", 8 ,bytes_transferred);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		return;
	}

	//��֤��ͷ

	//��֤��ͷ��ʼ��־
	unsigned char btBegin = m_pchHead[0];
	if (btBegin != 0x55)
	{
		//��������֪ͨ
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
		}

		disconnect_handler(sock,E_WN_CLIENT_RECV_HEAD_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_HEAD_FAIL;
#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_head_handler] [��ʼ��־ = %d]",btBegin);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		return;
	}

	//��֤��ͷЭ��汾
	unsigned char btVersion = m_pchHead[1];
	if (btVersion != 1)
	{
		//��������֪ͨ
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
		}

		disconnect_handler(sock,E_WN_CLIENT_RECV_HEAD_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_HEAD_FAIL;
#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_head_handler] [Э��汾 = %d]", btVersion);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		return;
	}

	//��֤������������
	unsigned char btType = m_pchHead[2];
	if (btType != 1 && btType != 2 && btType != 3)
	{
		//��������֪ͨ
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
		}

		disconnect_handler(sock,E_WN_CLIENT_RECV_HEAD_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_HEAD_FAIL;

#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_head_handler] [������������ = %d]", btType);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		return;
	}

	//��֤��������ѹ������
	unsigned char btCompress = m_pchHead[3];
	if (btCompress != 0 && btCompress != 1 && btCompress != 2)
	{
		//��������֪ͨ
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
		}

		disconnect_handler(sock,E_WN_CLIENT_RECV_HEAD_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_HEAD_FAIL;
#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_head_handler] [��������ѹ������ = %d]", btCompress);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		return;
	}

	//ȡ�ð������ݳ���
	unsigned int ulLength;
	memcpy(&ulLength, m_pchHead + 4, sizeof(ulLength));

	if (ulLength + 2 > (unsigned long)m_nMaxBodyLength)
	{
#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_head_handler] [���ӽ��հ��建���� ����ǰ[%d] ���Ӻ�[%d]]", m_nMaxBodyLength, ulLength + 2);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

		m_nMaxBodyLength = ulLength + 2;

		if (m_pchBody)
		{
			delete[] m_pchBody;
			m_pchBody = NULL;
		}

		try
		{
			m_pchBody = new unsigned char[m_nMaxBodyLength];
		}
		catch (std::bad_alloc& e)
		{
#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CConnection::recv_head_handler] [���ջ������ʧ��ClientID=%llu, Error = %s]", (unsigned __int64)this, e.what());
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}

		if (!m_pchBody)
		{
			disconnect_handler(sock,E_WN_CLIENT_RECV_HEAD_FAIL);
			m_nErrorCode = E_WN_CLIENT_RECV_HEAD_FAIL;

#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CConnection::recv_head_handler] [�ڴ����ʧ��ClientID=%llu]",(unsigned __int64)this);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;

		}
	}

	//��������
	if ( sock )
	{
		//�첽���հ���
		//boost::shared_ptr<unsigned char> pbtBody(new unsigned char[ulLength + 2], deleter);
		async_read(*sock, boost::asio::buffer(m_pchBody, ulLength + 2), m_strand->wrap(boost::bind(&CConnection::recv_body_handler,shared_from_this(),
			sock, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, btType,/* pbtBody,*/ ulLength)));
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_head_handler] [OUT]\n");
#endif
}


void CConnection::RecvNOTMDFData( sock_pt sock )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::RecvNOTMDFData] [IN]");
#endif

	if (!m_bConnect || m_bAppExit)
	{
		return;
	}

	if ( !sock || !m_strand )
	{
		disconnect_handler(sock,E_WN_CLIENT_RECV_NOMDF_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_NOMDF_FAIL;
		return;
	}

	//boost::shared_ptr<unsigned char> btBuffer(new unsigned char[1048576], deleter);
	//sock->async_receive( boost::asio::buffer(&*btBuffer,1048576),m_strand->wrap(boost::bind(&CConnection::recv_NOT_MDF_handler, shared_from_this(), sock, btBuffer, 
	//	boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ));

	sock->async_receive( boost::asio::buffer(m_pchBody, MAX_BODY_LENGTH), m_strand->wrap(boost::bind(&CConnection::recv_NOT_MDF_handler, shared_from_this(), sock,
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) ));

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::RecvNOTMDFData] [OUT]\n");
#endif
}

//void CConnection::recv_NOT_MDF_handler( sock_pt sock, boost::shared_ptr<unsigned char> btBuffer, const boost::system::error_code& ec, std::size_t bytes_transferred )
void CConnection::recv_NOT_MDF_handler( sock_pt sock, const boost::system::error_code& ec, std::size_t bytes_transferred )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_NOT_MDF_handler] [IN]");
#endif

	if (!m_bConnect || m_bAppExit)
	{
		return;
	}

	if ( ec || !m_socket || !m_strand || !sock )
	{
		disconnect_handler(sock,E_WN_CLIENT_RECV_NOMDF_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_NOMDF_FAIL;
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CConnection::recv_NOT_MDF_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		return;
	} 

	CTigerClient* pCommonClient = NULL;
	if (m_pClient)
	{
		pCommonClient = static_cast<CTigerClient*>(m_pClient);
	}

	//���ݰ�֪ͨ
	if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
	{
		//pCommonClient->m_pTCPClientNotifyObj->OnRecvServerData( &*btBuffer, bytes_transferred );
		pCommonClient->m_pTCPClientNotifyObj->OnRecvServerData( m_pchBody, bytes_transferred );
	}

	//������������
	if ( sock )
	{
		//sock->async_receive( boost::asio::buffer(&*btBuffer,1048576), m_strand->wrap(boost::bind(&CConnection::recv_NOT_MDF_handler,
		//	shared_from_this(), sock, btBuffer, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred )) );

		sock->async_receive( boost::asio::buffer(m_pchBody, MAX_BODY_LENGTH), m_strand->wrap(boost::bind(&CConnection::recv_NOT_MDF_handler,
			shared_from_this(), sock, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred )) );
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_NOT_MDF_handler] [OUT]");
#endif
}

//==================================================================
//��������  recv_body_handler
//���ߣ� yin hui   
//���ڣ� 20110128    
//���ܣ�  ���հ���Ļص�����  
//���������const system::error_code& ec,	������
//			unsigned char btType,					������������
//			shared_ptr<unsigned char> btBody,		��������
//			unsigned int ulLength					�������ݳ���
//����ֵ�� �� 
//�޸ļ�¼����
//==================================================================
void CConnection::recv_body_handler( sock_pt sock, const boost::system::error_code& ec, size_t bytes_transferred, unsigned char btType,
									  /*boost::shared_ptr<unsigned char> btBody,*/ unsigned int ulLength)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_body_handler] [IN]");
#endif

	if (!m_bConnect || m_bAppExit)
	{
		return;
	}

	if (ec || !m_socket || !sock || !m_strand )
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CConnection::recv_body_handler] [recv body error]");
#endif

		disconnect_handler(sock,E_WN_CLIENT_RECV_BODY_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_BODY_FAIL;
#ifdef __LOG__
		char szLog[1024];
		std::string err = ec.message();
		sprintf(szLog, "[CConnection::recv_body_handler] [%s]",err.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_body_handler] [OUT]\n");
#endif
		return;
	} 

	CTigerClient* pCommonClient = NULL;
	if (m_pClient)
	{
		pCommonClient = static_cast<CTigerClient*>(m_pClient);
	}

	//�޸�������ʱ��
	m_tLastRecv = (long)time(0);

	//����β
	unsigned char btCheckSum;
	unsigned char btSuffix;
	memcpy(&btCheckSum, m_pchBody + ulLength, 1);
	memcpy(&btSuffix, m_pchBody + ulLength + 1, 1);

	//��֤У���
	unsigned char chCheckSum;
	if (btType == 2)
	{
		chCheckSum = 0;
	} 
	else
	{
		if (bytes_transferred != ulLength + 2)
		{
			//��������֪ͨ
			if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
			{
				pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
			}

			disconnect_handler(sock,E_WN_CLIENT_RECV_BODY_FAIL);
			m_nErrorCode = E_WN_CLIENT_RECV_BODY_FAIL;

#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::recv_body_handler] [�����ֽ������� Ӧ����[%d] �Ѵ���[%d]]", ulLength + 2, bytes_transferred);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
			CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_body_handler] [OUT]\n");
#endif
			return;
		}

		//memcpy_s(pCRCBuffer, MAX_BODY_LENGTH, &(*btBody), ulLength);
		//memcpy(pCRCBuffer, &(*btBody), ulLength);
		m_crc32.reset();
		m_crc32.process_bytes(m_pchBody, ulLength);
		chCheckSum = m_crc32();
	}

	if (btCheckSum != chCheckSum)
	{
		//��������֪ͨ
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();		
		}

		disconnect_handler(sock,E_WN_CLIENT_RECV_BODY_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_BODY_FAIL;

#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_body_handler] [����ǰУ��� = %d, �����У��� = %d, ���� = %d]", 
			btCheckSum, chCheckSum, ulLength);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_body_handler] [OUT]\n");
#endif
		return;
	}

	//��֤������־
	if (btSuffix != 0x16)
	{
		//��������֪ͨ
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
		}

		disconnect_handler(sock,E_WN_CLIENT_RECV_BODY_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_BODY_FAIL;

#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_body_handler] [������־=%d]",btSuffix);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_body_handler] [OUT]\n");
#endif
		return;
	}

	if (btType == 1)  //���ݰ�
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_body_handler] [OnrecvServerData begin]\n");
#endif
		//���ݰ�֪ͨ
		//if (!m_bAppExit)//��ΪCloseSocket�˳�ʱҪ�ȴ������Ҵ�ʱҪ֪ͨ���ݵ��������MFC�ĳ��򣬴�ʱ����ֽ��潩������Ϊ��ʱ��һ����Ϣû������
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnRecvServerData(m_pchBody,ulLength);
		}
	} 
	else if (btType == 2)	//������
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_body_handler] [��������]");
#endif
	}
	else if (btType == 3)	//��¼��֤��Ϣ
	{

	}
	else //��������
	{
		//��������֪ͨ
		//if (!m_bAppExit)//��ΪCloseSocket�˳�ʱҪ�ȴ������Ҵ�ʱҪ֪ͨ�������ݣ�������MFC�ĳ��򣬴�ʱ����ֽ��潩������Ϊ��ʱ��һ����Ϣû������
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
		}
	}
	
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_body_handler] [recv next]\n");
#endif

	//����������һ����ͷ
	//boost::shared_ptr<unsigned char> btHead(new unsigned char[8], deleter);
	async_read(*sock, boost::asio::buffer(m_pchHead, 8),  m_strand->wrap(boost::bind(&CConnection::recv_head_handler, shared_from_this() ,
		sock, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred/*, btHead*/)));

	//btBody.reset();

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_body_handler] [OUT]\n");
#endif
}

//==================================================================
//�������� do_close 
//���ߣ� yin hui   
//���ڣ� 20110223     
//���ܣ� �ر�socket  
//�����������
//����ֵ��  ��
//�޸ļ�¼����
//==================================================================
void CConnection::do_close(bool bExitFlag )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::do_close] [IN]");
#endif

	if (bExitFlag)
	{
		m_bAppExit = true;
		m_nErrorCode = E_WN_CLIENT_APP_EXIT;
	}
	else
	{
		m_nErrorCode = E_WN_CLIENT_CLOSE_SOCKET;
	}

	disconnect_handler( m_socket, m_nErrorCode );

	if ( m_pIO )
	{
		m_pIO->stop();
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::do_close] [OUT]\n");
#endif
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
void CConnection::CloseSocket( bool bExitFlag )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::CloseSocket] [IN]");
#endif

	if (bExitFlag)
	{
		m_bAppExit = true;
		m_nErrorCode = E_WN_CLIENT_APP_EXIT;
	}
	else
	{
		m_nErrorCode = E_WN_CLIENT_CLOSE_SOCKET;
	}

	CTigerClient* pCommonClient = NULL;
	if (m_pClient)
	{
		pCommonClient = static_cast<CTigerClient*>(m_pClient);
	}

	//if (m_bConnect && !m_bAppExit)
	//{
	//	if ( m_pIO && m_strand )
	//	{
	//		m_pIO->post(m_strand->wrap(boost::bind(&CConnection::do_close, shared_from_this(), bExitFlag)));
	//	}
	//	else
	//	{
	//		if ( pCommonClient && pCommonClient->m_pTCPClientNotifyObj )
	//		{
	//			pCommonClient->m_pTCPClientNotifyObj->OnServerDisconnect(m_nErrorCode);
	//		}
	//	}
	//}
	//else
	//{
	//	if ( pCommonClient && pCommonClient->m_pTCPClientNotifyObj )
	//	{
	//		pCommonClient->m_pTCPClientNotifyObj->OnServerDisconnect(m_nErrorCode);
	//	}
	//}

	if (m_bConnect) //�Ѿ�������
	{
		if ( m_pIO && m_strand )
		{
			m_pIO->post(m_strand->wrap(boost::bind(&CConnection::do_close, shared_from_this(), bExitFlag)));
		}
		else
		{
			if ( pCommonClient && pCommonClient->m_pTCPClientNotifyObj )
			{
				pCommonClient->m_pTCPClientNotifyObj->OnServerDisconnect(m_nErrorCode);
			}
		}
	}
	else //û��������
	{
		if ( pCommonClient && pCommonClient->m_pTCPClientNotifyObj )
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDisconnect(m_nErrorCode);
		}
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::CloseSocket] [OUT]\n");
#endif
}


//==================================================================
//��������  disconnect_handler
//���ߣ� yin hui   
//���ڣ� 20110128     
//���ܣ�  ������  
//���������int nErrorCode ������
//����ֵ��  ��
//�޸ļ�¼����
//==================================================================
void CConnection::disconnect_handler(sock_pt sock, int nErrorCode)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::disconnect_handler] [IN]");
#endif

	m_bConnect = false;

	if ( !m_listSendData.empty() )
		m_listSendData.clear();

	if ( !m_listSendData_NonMdf.empty() )
		m_listSendData_NonMdf.clear();

	//�����˳�ʱ���Ȳ��ر�socket�������������йر�
	if (m_pIO /*&& nErrorCode != E_WN_CLIENT_OTHER_ERROR*/)
	{
		//if(m_ReadTimer)
		//{
		//	m_ReadTimer->cancel();
		//	m_ReadTimer.reset();
		//}

		if (m_ConnectTimer)
		{
			m_ConnectTimer->cancel();
			m_ConnectTimer.reset();
		}

		if (m_HBTimer)
		{
			m_HBTimer->cancel();
			m_HBTimer.reset();
		}

		if (m_HBCheckTimer)
		{
			m_HBCheckTimer->cancel();
			m_HBCheckTimer.reset();
		}

		if (m_socket)
		{
			m_socket->close();
			m_socket.reset();
		}
	}


#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::disconnect_handler] [OUT]\n");
#endif
}

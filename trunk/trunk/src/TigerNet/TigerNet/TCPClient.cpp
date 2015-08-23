
#include "stdafx.h"
#include "TCPClient.h"

#include "TigerServer.h"

//==================================================================
//�������� CTCPClient 
//���ߣ� yin hui   
//���ڣ� 20110128    
//���ܣ�  CTCPClient���캯������ʼ��  
//���������io_service& io			�첽��Ϣѭ��
//			sock_pt sock			���ݽ��շ��͵�socket
//			ITCPServer* pServer		������ʵ��
//����ֵ��  ��
//�޸ļ�¼��20100228 ��ʼ��m_tLastRecv
//==================================================================
CTCPClient::CTCPClient(boost::asio::io_service& io,boost::shared_ptr<boost::asio::ip::tcp::socket> sock,ITCPServer* pServer, boost::asio::strand* pStrand )
:m_HBTimer(new boost::asio::deadline_timer(io)),m_HBCheckTimer(new boost::asio::deadline_timer(io))/*,m_strand(new boost::asio::strand(io))*/
,m_ptrSock(sock)
{
	m_pServer = static_cast<ITCPServer*>(pServer);
	m_bHBThread = false;
	m_bRecvThread = false;
	m_bExit = false;
	m_tLastRecvHB = (long)time(0);
	m_tLastSend = (long)time(0);

	m_tLastRecv = (long)time(0);

	m_pchHead = new unsigned char[HEADER_LENGTH];
	m_pchBody = new unsigned char[MAX_BODY_LENGTH];
	m_pRecvBuffer = new unsigned char[RECV_BUFFER_SIZE];

	m_nMaxBodyLength = MAX_BODY_LENGTH;

	m_strand = new boost::asio::strand(io);
}

//==================================================================
//��������~CTCPClient  
//���ߣ� yin hui   
//���ڣ� 20110128    
//���ܣ� CTCPClient��������   
//�����������
//����ֵ��  ��
//�޸ļ�¼����
//==================================================================
CTCPClient::~CTCPClient()
{
	m_bExit = true;

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

	//if (pCRCBuffer)
	//{
	//	delete[] pCRCBuffer;
	//	pCRCBuffer = NULL;
	//}
}

void CTCPClient::Start( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, bool bMDF )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::Start] [IN]");
#endif

	if ( bMDF )
	{
		//����������ⶨʱ��
		if (m_HBTimer)
		{
			m_HBTimer->expires_from_now(boost::posix_time::seconds(HB_SEND_PERIOD));
			m_HBTimer->async_wait( bind(&CTCPClient::HB_timer_handler, shared_from_this(),sock,boost::asio::placeholders::error));
		}

		//�����������
		if (m_HBCheckTimer)
		{
			m_HBCheckTimer->expires_from_now(boost::posix_time::seconds(3 * HB_SEND_PERIOD));
			m_HBCheckTimer->async_wait( bind(&CTCPClient::check_hb_timer_handler, shared_from_this(), sock, boost::asio::placeholders::error));
		}
	}

	//��ʼ��������
	m_bRecvThread = true;
	RecvData( sock, bMDF );

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::Start] [OUT]");
#endif
}


void CTCPClient::check_hb_timer_handler(boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec)
{
#ifdef __LOG__
	char szLog[1024];
	sprintf(szLog, "[CTCPClient::check_hb_timer_handler] [IN] [ClientID = %llu]", (__int64)this);
	CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

	if (m_bExit)
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::check_hb_timer_handler] [�ͻ����Ѿ��˳�] [OUT]");
#endif
		return;
	}

	//��ʱ����ȡ��
	if ( ec )
	{
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CTCPClient::check_hb_timer_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::check_hb_timer_handler] [OUT]");
#endif
		return;
	}

	long lTime = (long)time(0);
	if (lTime - m_tLastRecv < 45)
	{
		//���̷߳�ʽ�����ܵ���sleep
		if (m_HBCheckTimer)
		{
			m_HBCheckTimer->expires_from_now(boost::posix_time::seconds(45 - (lTime - m_tLastRecv)));
			m_HBCheckTimer->async_wait( bind(&CTCPClient::check_hb_timer_handler, shared_from_this(), sock, boost::asio::placeholders::error));
		}

	}
	else //����45sû�н��յ�����
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CTCPClient::check_hb_timer_handler] [����45��û���յ�����]");
#endif

		if (!m_bExit)
		{
			disconnect_handler( sock, E_WN_SERVER_RECV_NO_HB );
		}
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::check_hb_timer_handler] [OUT]");
#endif
}

void CTCPClient::send_hb_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec )
{
#ifdef __LOG__
	char szLog[1024];
	sprintf(szLog, "[CTCPClient::send_hb_handler] [ClientID = %llu]", (unsigned __int64)this);
	CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
	{
		if (m_bExit)
		{
#ifdef __LOG__
			CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::send_hb_handler] [�ͻ����Ѿ��˳�] [OUT]");
#endif
			return;
		}
	}

	if (ec)
	{
		if (!m_bExit)
		{
			disconnect_handler(sock,E_WN_SERVER_SEND_HB_FAIL);
		}

#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CTCPClient::send_hb_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
	} 
	else
	{
		m_tLastSend = (long)time(0);
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::send_hb_handler] [OUT]");
#endif
}

void CTCPClient::RecvData( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, bool bMDF )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::RecvData] [IN]");
#endif

	if (m_bExit)
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::RecvData] [�ͻ����Ѿ��˳�] [OUT]");
#endif
		return;
	}

	if ( bMDF )//MDFЭ������ݽ���
	{
		//�첽���հ�ͷ
		if(sock->is_open())
		{
			async_read(*sock, boost::asio::buffer(m_pchHead,8), 
				
				boost::bind(&CTCPClient::recv_head_handler,shared_from_this(), sock, boost::asio::placeholders::error, m_pchHead)) ;
		}

	}
	else	//��MDFЭ�飬û��������ⶨʱ��
	{
		//�첽��������
		if (sock->is_open())
		{
			try
			{
				//boost::shared_ptr<unsigned char> btData(new unsigned char[1024], deleter);

				sock->async_receive(boost::asio::buffer(&*m_pRecvBuffer, RECV_BUFFER_SIZE), 
					boost::bind(&CTCPClient::recv_non_mdf_handler, shared_from_this(), sock, boost::asio::placeholders::error, m_pRecvBuffer, boost::asio::placeholders::bytes_transferred)
					);
			}
			catch (std::bad_alloc& e)
			{
#ifdef __LOG__
				char szLog[1024];
				sprintf(szLog, "[CTCPClient::RecvData] [NonMDF���ջ������ʧ��ClientID=%llu, Error = %s]", (unsigned __int64)this, e.what());
				CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
				return;
			}
		}

	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::RecvData] [OUT]");
#endif
}

void CTCPClient::recv_non_mdf_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec, 
									  const unsigned char* btData, size_t bytes_transferred)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::recv_non_mdf_handler] [IN]");
#endif

	if (m_bExit)
	{
		return;
	}

	if (ec)
	{

		if (!m_bExit)
		{
			disconnect_handler( sock, E_WN_SERVER_RECV_NOMDF_FAIL );
		}

#ifdef __LOG__		
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CTCPClient::recv_non_mdf_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
	} 
	else
	{
		//�޸�������ʱ��
		m_tLastRecv = (long)time(0);

		CTigerServer* pCommonServer = NULL;
		if (m_pServer)
		{
			pCommonServer = static_cast<CTigerServer*>(m_pServer);
			pCommonServer->m_pTCPServerNotifyObj->OnRecvClientData((unsigned __int64)this, btData, bytes_transferred);
		}

		//�첽��������
		try
		{
			//boost::shared_ptr<unsigned char> btData(new unsigned char[1024], deleter);
		}
		catch (std::bad_alloc& e)
		{
#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::recv_non_mdf_handler] [NonMDF���ջ������ʧ��ClientID=%llu, Error = %s]", (unsigned __int64)this, e.what());
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}

		sock->async_receive(boost::asio::buffer(&*m_pRecvBuffer, RECV_BUFFER_SIZE), boost::bind(&CTCPClient::recv_non_mdf_handler,shared_from_this(), sock, 
			boost::asio::placeholders::error, m_pRecvBuffer, boost::asio::placeholders::bytes_transferred));
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::recv_non_mdf_handler] [OUT]");
#endif
}


void CTCPClient::recv_head_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec, unsigned char * pucHead)
{
#ifdef __LOG__
	char szLog[1024];
	sprintf(szLog, "[CTCPClient::recv_head_handler] [IN] [ClientID = %llu]", (unsigned __int64)this);
	CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

	if (m_bExit)
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::recv_head_handler] [�ͻ����Ѿ��˳�]");
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::recv_head_handler] [OUT]");
#endif
		return;
	}

	if (ec)
	{
		if (!m_bExit)
		{
			disconnect_handler( sock, E_WN_SERVER_RECV_HEAD_FAIL );
		}

#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CTCPClient::recv_head_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
	} 
	else
	{
		CTigerServer* pCommonServer = NULL;
		if (m_pServer)
		{
			pCommonServer = static_cast<CTigerServer*>(m_pServer);
		}

		if (pCommonServer && pCommonServer->m_pIO)
		{
		}
		else
		{
			return;
		}

		//��֤��ͷ

		//��֤��ͷ��ʼ��־
		unsigned char btBegin = m_pchHead[0];
		if (btBegin != 0x55)
		{
			//cout << "wrong prefix" << endl;

			//��������֪ͨ
			pCommonServer->m_pTCPServerNotifyObj->OnClientDataError((unsigned __int64)this);

			{
				//boost::mutex::scoped_lock lock(m_exit_mutex);
				if (!m_bExit)
				{
					disconnect_handler(sock,E_WN_SERVER_RECV_HEAD_FAIL);
				}
			}

#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::recv_head_handler] [��ʼ��־=%d]",btBegin);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}

		//��֤��ͷЭ��汾
		unsigned char btVersion = m_pchHead[1];
		if (btVersion != 1)
		{
			//��������֪ͨ
			pCommonServer->m_pTCPServerNotifyObj->OnClientDataError((unsigned __int64)this);

			if (!m_bExit)
			{
				disconnect_handler(sock, E_WN_SERVER_RECV_HEAD_FAIL);
			}

#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::recv_head_handler] [Э��汾=%d]",btVersion);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}

		//��֤������������
		unsigned char btType = m_pchHead[2];
		if (btType != 1 && btType != 2 && btType != 3)
		{
			//cout << "wrong type" << endl;

			//��������֪ͨ
			pCommonServer->m_pTCPServerNotifyObj->OnClientDataError((unsigned __int64)this);

			{
				//boost::mutex::scoped_lock lock(m_exit_mutex);
				if (!m_bExit)
				{
					disconnect_handler(sock,E_WN_SERVER_RECV_HEAD_FAIL);
				}
			}

#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::recv_head_handler] [������������=%d]",btType);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}

		//��֤��������ѹ������
		unsigned char btCompress = m_pchHead[3];
		if (btCompress != 0 && btCompress != 1 && btCompress != 2)
		{
			//cout << "wrong prefix" << endl;

			//��������֪ͨ
			pCommonServer->m_pTCPServerNotifyObj->OnClientDataError((unsigned __int64)this);

			{
				//boost::mutex::scoped_lock lock(m_exit_mutex);
				if (!m_bExit)
				{
					disconnect_handler( sock, E_WN_SERVER_RECV_HEAD_FAIL );
				}
			}

#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::recv_head_handler] [��������ѹ������=%d]",btCompress);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}

		//ȡ�ð������ݳ���
		unsigned long ulLength;
		memcpy(&ulLength, m_pchHead + 4,sizeof(ulLength));

		if (ulLength + 2 > (unsigned long)m_nMaxBodyLength)
		{
#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::recv_body_handler] [���ӽ��հ��建���� ����ǰ[%d] ���Ӻ�[%d]]", m_nMaxBodyLength, ulLength + 2);
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
				sprintf(szLog, "[CTCPClient::recv_head_handler] [���ջ������ʧ��ClientID=%llu, Error = %s]", (unsigned __int64)this, e.what());
				CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
				return;
				
			}

			
			if (!m_pchBody)
			{
				{
					//boost::mutex::scoped_lock lock(m_exit_mutex);
					if (!m_bExit)
					{
						disconnect_handler( sock, E_WN_SERVER_RECV_HEAD_FAIL );
					}
				}

#ifdef __LOG__
				char szLog[1024];
				sprintf(szLog, "[CTCPClient::recv_head_handler] [�ڴ����ʧ��ClientID=%llu]",(unsigned __int64)this);
				CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
				return;

			}
		}

		//�첽���հ���
		if (sock->is_open())
		{
			async_read(*sock, boost::asio::buffer(m_pchBody,ulLength + 2), bind(&CTCPClient::recv_body_handler, shared_from_this(), sock, boost::asio::placeholders::error, 
				boost::asio::placeholders::bytes_transferred, btType, ulLength));
		}

	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::recv_head_handler] [OUT]");
#endif
}

void CTCPClient::recv_body_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec,size_t bytes_transferred,unsigned char btType,unsigned long ulLength)
{
#ifdef __LOG__
	char szLog[1024];
	sprintf(szLog, "[CTCPClient::recv_body_handler] [ClientID = %llu]", (unsigned __int64)this);
	CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

	if (m_bExit)
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::recv_body_handler] [�ͻ����Ѿ��˳�]");
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::recv_body_handler] [OUT]");
#endif
		return;
	}

	if (ec)
	{
		std::string str = ec.message();
		if ( !m_bExit )
		{
			disconnect_handler( sock, E_WN_SERVER_RECV_BODY_FAIL);
		}

#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CTCPClient::recv_body_handler] [%s]", str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR, szLog);
#endif
	} 
	else
	{
		CTigerServer* pCommonServer = NULL;

		{
			//boost::mutex::scoped_lock lock(m_exit_mutex);
			if (m_pServer)
			{
				pCommonServer = static_cast<CTigerServer*>(m_pServer);

				if (pCommonServer && pCommonServer->m_pIO)
				{

				}
				else
				{
					return;
				}
			}
		}

		//�޸�������ʱ��
		m_tLastRecv = (long)time(0);

		//����β
		unsigned char btCheckSum;
		unsigned char btSuffix;
		memcpy(&btCheckSum,m_pchBody + ulLength,1);
		memcpy(&btSuffix, m_pchBody + ulLength + 1,1);

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
				pCommonServer->m_pTCPServerNotifyObj->OnClientDataError((unsigned __int64)this);

				{
					//boost::mutex::scoped_lock lock(m_exit_mutex);
					if (!m_bExit)
					{
						disconnect_handler(sock,E_WN_SERVER_RECV_BODY_FAIL);
					}
				}

#ifdef __LOG__
				char szLog[1024];
				sprintf(szLog, "[CTCPClient::recv_body_handler] [�����ֽ������� Ӧ����[%d] �Ѵ���[%d]]", ulLength + 2, bytes_transferred);
				CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
				return;

			}

			//memcpy_s(pCRCBuffer, MAX_BODY_LENGTH, &(*pbtBody), ulLength);
			m_crc32.reset();
			m_crc32.process_bytes(m_pchBody,ulLength);
			chCheckSum = m_crc32();
		}

		if (btCheckSum != chCheckSum)
		{
			//��������֪ͨ
			pCommonServer->m_pTCPServerNotifyObj->OnClientDataError((unsigned __int64)this);

			{
				//boost::mutex::scoped_lock lock(m_exit_mutex);
				if (!m_bExit)
				{
					disconnect_handler( sock, E_WN_SERVER_RECV_BODY_FAIL );
				}
			}

#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::recv_body_handler] [����ǰУ��� = %d, �����У��� = %d]", btCheckSum, chCheckSum);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}

		//��֤������־
		if (btSuffix != 0x16)
		{
			//��������֪ͨ
			pCommonServer->m_pTCPServerNotifyObj->OnClientDataError((unsigned __int64)this);

			{
				//boost::mutex::scoped_lock lock(m_exit_mutex);
				if (!m_bExit)
				{
					disconnect_handler(sock, E_WN_SERVER_RECV_BODY_FAIL);
				}
			}

#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::recv_body_handler] [������־=%d]",btSuffix);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}

		if (btType == 1)	//���ݰ�
		{
			pCommonServer->m_pTCPServerNotifyObj->OnRecvClientData((unsigned __int64)this, m_pchBody, ulLength);
		} 
		else if (btType == 2)	//������
		{
#ifdef __LOG__
			CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::recv_body_handler] [��������]");
#endif
		}
		else if (btType == 3)	//��¼��֤��Ϣ
		{

		}
		else //��������
		{
			//��������֪ͨ
			pCommonServer->m_pTCPServerNotifyObj->OnClientDataError((unsigned __int64)this);

			{
				//boost::mutex::scoped_lock lock(m_exit_mutex);
				if (!m_bExit)
				{
					disconnect_handler( sock, E_WN_SERVER_RECV_BODY_FAIL );
				}
			}

#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::recv_body_handler] [%d]",btType);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}

		//����������һ����ͷ
		if ( sock->is_open() )
		{
			//shared_ptr<unsigned char> btHead(new unsigned char[8]);
			async_read(*sock, boost::asio::buffer(m_pchHead,8), bind(&CTCPClient::recv_head_handler,shared_from_this(), sock, 
				boost::asio::placeholders::error, m_pchHead));
		}

	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::recv_body_handler] [OUT]");
#endif
}

void CTCPClient::disconnect_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, int nErrorCode )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::disconnect_handler] [IN]");
#endif

	m_bExit = true;

	if ( !m_listSendData.empty() )
		m_listSendData.clear();

	if ( !m_listSendData_NonMdf.empty() )
		m_listSendData_NonMdf.clear();

	if ( m_ptrSock )
	{
		m_ptrSock->close();
		m_ptrSock.reset();
	}

	m_bHBThread = false;
	m_bRecvThread = false;

	CTigerServer* pCommonServer = NULL;

	if (m_pServer)
	{
		pCommonServer = static_cast<CTigerServer*>(m_pServer);
		if ( pCommonServer && pCommonServer->m_pIO && nErrorCode != E_WN_SERVER_NOTIFY_CLOSE)
		{
			if (m_HBTimer)
			{
				m_HBTimer->cancel();
				delete m_HBTimer;
				m_HBTimer = NULL;
			}

			if (m_HBCheckTimer)
			{
				m_HBCheckTimer->cancel();
				delete m_HBCheckTimer;
				m_HBCheckTimer = NULL;
			}

			//if (m_strand)
			//{
			//	delete m_strand;
			//	m_strand = NULL;
			//}
		}
		else
		{
			return;
		}
	}
	else
	{
		return;
	}

	switch (nErrorCode)
	{
	case E_WN_SERVER_SEND_DATA_FAIL:
	case E_WN_SERVER_SEND_HB_FAIL:
	case E_WN_SERVER_RECV_HEAD_FAIL:
	case E_WN_SERVER_RECV_BODY_FAIL:
	case E_WN_SERVER_RECV_TIMEOUT:
	case E_WN_SERVER_RECV_NOMDF_FAIL:
	case E_WN_SERVER_RECV_NO_HB:
		if ( pCommonServer )
		{
			pCommonServer->server_disconnect_handler((unsigned __int64)this, sock, nErrorCode);
		}
		break;

	case E_WN_SERVER_NOTIFY_CLOSE:
		break;

	default :
		break;
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::disconnect_handler] [OUT]");
#endif
}


void CTCPClient::CloseSocket()
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::CloseSocket] [IN]");
#endif

	disconnect_handler( m_ptrSock, E_WN_SERVER_NOTIFY_CLOSE );

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::CloseSocket] [OUT]\n");
#endif
}

void CTCPClient::send_mdfdata_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec, packet_pt pPacket )
{
#ifdef __LOG__
	char szLog[1024];
	sprintf(szLog, "[CTCPClient::send_mdfdata_handler] [ClientID = %llu]", (unsigned __int64)this);
	CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

	if (m_bExit)
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::send_mdfdata_handler] [OUT]");
#endif
		return;
	}

	if(ec)
	{
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CTCPClient::send_mdfdata_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

		if (!m_bExit)
		{
			disconnect_handler( sock, E_WN_SERVER_SEND_DATA_FAIL);
		}
	}
	else
	{
		//���������ʱ��
		m_tLastSend = (long)time(0);

		m_listSendData.pop_front();
		if ( !m_listSendData.empty() )
		{
			packet_pt pNextPacket = m_listSendData.front();
			if ( m_ptrSock->is_open() )
			{
#ifdef __LOG__
				char szLog[1024];
				sprintf(szLog, "[CTCPClient::send_mdfdata_handler] [���Ͷ��г��� = %d]", m_listSendData.size());
				CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

				async_write(*m_ptrSock, boost::asio::buffer(pNextPacket->GetBuffer(), pNextPacket->GetSize()), 
					boost::bind(&CTCPClient::send_mdfdata_handler, shared_from_this(), m_ptrSock, boost::asio::placeholders::error, pNextPacket));
			}
		}
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::send_mdfdata_handler] [OUT]");
#endif
}

//==================================================================
//��������  send_nonmdfdata_handler
//���ߣ� yin hui   
//���ڣ� 20110128      
//���ܣ� ����nonmdf���ݵĻص�����   
//���������const system::error_code& ec	������
//����ֵ��  ��
//�޸ļ�¼����
//==================================================================
void CTCPClient::send_nonmdfdata_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec, nonmdfpacket_pt pPacket)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::send_nonmdfdata_handler] [IN]");
#endif

	if (m_bExit)
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::send_nonmdfdata_handler] [�ͻ����Ѿ��˳�] [OUT]");
#endif
		return;
	}

	if(ec)
	{
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CTCPClient::send_nonmdfdata_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

		if (!m_bExit)
		{
			disconnect_handler( sock, E_WN_SERVER_SEND_DATA_FAIL);
		}
	}
	else
	{
		//���������ʱ��
		m_tLastSend = (long)time(0);

		pPacket.reset();

		m_listSendData_NonMdf.pop_front();

		if ( !m_listSendData_NonMdf.empty() )
		{
#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::send_nonmdfdata_handler] [���Ͷ��г��� = %d]", m_listSendData_NonMdf.size());
			CLogWriter::Log(WN_LOG_LEVEL_ERROR, szLog);
#endif

			nonmdfpacket_pt pNextPacket = m_listSendData_NonMdf.front();
			if ( m_ptrSock->is_open() )
			{
				//async_write(*m_ptrSock, boost::asio::buffer(pNextPacket->GetBuffer(),pNextPacket->GetSize()), bind(&CTCPClient::send_nonmdfdata_handler, shared_from_this(), 
				//	m_ptrSock, boost::asio::placeholders::error,pNextPacket)));

				m_strand->dispatch(bind(&CTCPClient::SendPacket, shared_from_this()));
			}
		}
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::send_nonmdfdata_handler] [OUT]");
#endif
}

void CTCPClient::SendPacket()
{
	//m_tLastSend = (long)time(0);

	if ( !m_listSendData_NonMdf.empty() )
	{
#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CTCPClient::SendPacket] [���Ͷ��г��� = %d]", m_listSendData_NonMdf.size());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR, szLog);
#endif

		nonmdfpacket_pt pNextPacket = m_listSendData_NonMdf.front();

		if ( m_ptrSock->is_open() )
		{
			async_write(*m_ptrSock, boost::asio::buffer(pNextPacket->GetBuffer(),pNextPacket->GetSize()), bind(&CTCPClient::send_nonmdfdata_handler, shared_from_this(), 
				m_ptrSock, boost::asio::placeholders::error,pNextPacket));
		}
	}

}

void CTCPClient::OnSendMDFData(sock_pt pSock, packet_pt pPacket)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::OnSendMDFData] [IN]");
#endif

	if (m_bExit)
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CTCPClient::OnSendMDFData] [�ͻ����Ѿ��˳�] [OUT]");
#endif
		return;
	}

	if ( m_listSendData.empty() )
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CTCPClient::OnSendMDFData] [���Ͷ���Ϊ�գ�ֱ�ӷ���]");
#endif
		if ( pSock->is_open() )
		{
			m_listSendData.push_back( pPacket );

			async_write(*pSock, boost::asio::buffer(pPacket->GetBuffer(),pPacket->GetSize()), bind(&CTCPClient::send_mdfdata_handler, shared_from_this(), 
				pSock, boost::asio::placeholders::error,pPacket));

			
		}		
	}
	else
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CTCPClient::OnSendMDFData] [���Ͷ��зǿգ��Ժ���]");
#endif
		m_listSendData.push_back( pPacket );
	}



#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::OnSendMDFData] [OUT]");
#endif
}


void CTCPClient::OnSendOtherData(sock_pt pSock, nonmdfpacket_pt pPacket)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::OnSendOtherData] [IN]");
#endif

	if (m_bExit)
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CTCPClient::OnSendOtherData] [�ͻ����Ѿ��˳�] [OUT]");
#endif
		return;
	}

	if ( m_listSendData_NonMdf.empty() )
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CTCPClient::OnSendOtherData] [���Ͷ���Ϊ�գ�ֱ�ӷ���]");
#endif
		if ( pSock->is_open() )
		{
			m_listSendData_NonMdf.push_back( pPacket );

			async_write(*pSock, boost::asio::buffer(pPacket->GetBuffer(),pPacket->GetSize()), bind(&CTCPClient::send_nonmdfdata_handler, shared_from_this(), 
				pSock, boost::asio::placeholders::error,pPacket));

			
		}		
	}
	else
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CTCPClient::OnSendOtherData] [���Ͷ��зǿգ��Ժ���]");
#endif

		int nSize = m_listSendData_NonMdf.size();
		if (nSize > 10000)
		{
			if (!m_bExit)
			{
				disconnect_handler( pSock, E_WN_SERVER_TOO_MUCH_DATA );
			}

#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::recv_head_handler] [���ݶѻ�,����[%d]", nSize);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

			return;
		}

		m_listSendData_NonMdf.push_back( pPacket );

//		//���������ʱ��
//		//m_tLastSend = (long)time(0);
//
//		//m_listSendData_NonMdf.pop_front();
//		if ( !m_listSendData_NonMdf.empty() )
//		{
//#ifdef __LOG__
//			char szLog[1024];
//			sprintf(szLog, "[CTCPClient::OnSendOtherData] [���Ͷ��г��� = %d]", m_listSendData_NonMdf.size());
//			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
//#endif
//
//			nonmdfpacket_pt pNextPacket = m_listSendData_NonMdf.front();
//			if ( m_ptrSock->is_open() )
//			{
//				async_write(*m_ptrSock, boost::asio::buffer(pNextPacket->GetBuffer(),pNextPacket->GetSize()),bind(&CTCPClient::send_nonmdfdata_handler, shared_from_this(), 
//					m_ptrSock, boost::asio::placeholders::error,pNextPacket));
//			}
//		}
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::OnSendOtherData] [OUT]");
#endif
}


void CTCPClient::HB_timer_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::HB_timer_handler] [IN]");
#endif

	{
		//boost::mutex::scoped_lock lock(m_exit_mutex);
		if (m_bExit)
		{
#ifdef __LOG__
			CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::HB_timer_handler] [�ͻ����Ѿ��˳�] [OUT]");
#endif
			return;
		}
	}

	//��ʱ����ȡ��
	if ( ec )
	{
		//std::cout << "timer is cancelled." << std::endl;
#ifdef __LOG__
		std::string str = ec.message();
		char szLog[1024];
		sprintf(szLog, "[CTCPClient::HB_timer_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
	}
	else
	{
		long lTime = (long)time(0);
		if (lTime - m_tLastSend < HB_SEND_PERIOD)
		{
			if (m_HBTimer)
			{
				//���̷߳�ʽ�����ܵ���sleep
				m_HBTimer->expires_from_now(boost::posix_time::seconds(HB_SEND_PERIOD - (lTime - m_tLastSend)));
				m_HBTimer->async_wait( bind(&CTCPClient::HB_timer_handler, shared_from_this(), sock, boost::asio::placeholders::error));
			}
		}
		else
		{
			if ( sock->is_open() )
			{

				try
				{
					packet_pt pHBPacket(new CPacket(2,0,NULL,1));

					//async_write(*sock,buffer(pHBPacket->GetBuffer(),pHBPacket->GetSize()),bind(&CTCPClient::send_hb_handler,shared_from_this(),sock,placeholders::error));
					CTigerServer* pCommonServer = static_cast<CTigerServer*>(m_pServer);
					if ( pCommonServer )
					{
						pCommonServer->m_pIO->post( bind(&CTCPClient::OnSendMDFData, shared_from_this(), sock, pHBPacket));
					}

					m_tLastSend = (long)time(0);

					if (m_HBTimer)
					{
						m_HBTimer->expires_from_now(boost::posix_time::seconds(HB_SEND_PERIOD));
						m_HBTimer->async_wait( bind(&CTCPClient::HB_timer_handler, shared_from_this(), sock, boost::asio::placeholders::error));
					}

				}
				catch (std::bad_alloc& e)
				{
#ifdef __LOG__
					char szLog[1024];
					sprintf(szLog, "[CTCPClient::HB_timer_handler] [����������ʧ��ClientID=%llu, Error = %s]", (unsigned __int64)this, e.what());
					CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
					return;
				}
				
			}
		}
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::HB_timer_handler] [OUT]");
#endif
}

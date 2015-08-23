
#include "stdafx.h"
#include "TCPClient.h"

#include "TigerServer.h"

//==================================================================
//函数名： CTCPClient 
//作者： yin hui   
//日期： 20110128    
//功能：  CTCPClient构造函数，初始化  
//输入参数：io_service& io			异步消息循环
//			sock_pt sock			数据接收发送的socket
//			ITCPServer* pServer		服务器实例
//返回值：  无
//修改记录：20100228 初始化m_tLastRecv
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
//函数名：~CTCPClient  
//作者： yin hui   
//日期： 20110128    
//功能： CTCPClient析构函数   
//输入参数：无
//返回值：  无
//修改记录：无
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
		//启动心跳检测定时器
		if (m_HBTimer)
		{
			m_HBTimer->expires_from_now(boost::posix_time::seconds(HB_SEND_PERIOD));
			m_HBTimer->async_wait( bind(&CTCPClient::HB_timer_handler, shared_from_this(),sock,boost::asio::placeholders::error));
		}

		//启动心跳检测
		if (m_HBCheckTimer)
		{
			m_HBCheckTimer->expires_from_now(boost::posix_time::seconds(3 * HB_SEND_PERIOD));
			m_HBCheckTimer->async_wait( bind(&CTCPClient::check_hb_timer_handler, shared_from_this(), sock, boost::asio::placeholders::error));
		}
	}

	//开始接收数据
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
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::check_hb_timer_handler] [客户端已经退出] [OUT]");
#endif
		return;
	}

	//定时器被取消
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
		//非线程方式，不能调用sleep
		if (m_HBCheckTimer)
		{
			m_HBCheckTimer->expires_from_now(boost::posix_time::seconds(45 - (lTime - m_tLastRecv)));
			m_HBCheckTimer->async_wait( bind(&CTCPClient::check_hb_timer_handler, shared_from_this(), sock, boost::asio::placeholders::error));
		}

	}
	else //超过45s没有接收到数据
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CTCPClient::check_hb_timer_handler] [超过45秒没有收到数据]");
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
			CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::send_hb_handler] [客户端已经退出] [OUT]");
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
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::RecvData] [客户端已经退出] [OUT]");
#endif
		return;
	}

	if ( bMDF )//MDF协议的数据接收
	{
		//异步接收包头
		if(sock->is_open())
		{
			async_read(*sock, boost::asio::buffer(m_pchHead,8), 
				
				boost::bind(&CTCPClient::recv_head_handler,shared_from_this(), sock, boost::asio::placeholders::error, m_pchHead)) ;
		}

	}
	else	//非MDF协议，没有心跳检测定时器
	{
		//异步接收数据
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
				sprintf(szLog, "[CTCPClient::RecvData] [NonMDF接收缓存分配失败ClientID=%llu, Error = %s]", (unsigned __int64)this, e.what());
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
		//修改最后接收时间
		m_tLastRecv = (long)time(0);

		CTigerServer* pCommonServer = NULL;
		if (m_pServer)
		{
			pCommonServer = static_cast<CTigerServer*>(m_pServer);
			pCommonServer->m_pTCPServerNotifyObj->OnRecvClientData((unsigned __int64)this, btData, bytes_transferred);
		}

		//异步接收数据
		try
		{
			//boost::shared_ptr<unsigned char> btData(new unsigned char[1024], deleter);
		}
		catch (std::bad_alloc& e)
		{
#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::recv_non_mdf_handler] [NonMDF接收缓存分配失败ClientID=%llu, Error = %s]", (unsigned __int64)this, e.what());
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
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::recv_head_handler] [客户端已经退出]");
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

		//验证包头

		//验证包头开始标志
		unsigned char btBegin = m_pchHead[0];
		if (btBegin != 0x55)
		{
			//cout << "wrong prefix" << endl;

			//错误数据通知
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
			sprintf(szLog, "[CTCPClient::recv_head_handler] [开始标志=%d]",btBegin);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}

		//验证包头协议版本
		unsigned char btVersion = m_pchHead[1];
		if (btVersion != 1)
		{
			//错误数据通知
			pCommonServer->m_pTCPServerNotifyObj->OnClientDataError((unsigned __int64)this);

			if (!m_bExit)
			{
				disconnect_handler(sock, E_WN_SERVER_RECV_HEAD_FAIL);
			}

#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::recv_head_handler] [协议版本=%d]",btVersion);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}

		//验证包体数据类型
		unsigned char btType = m_pchHead[2];
		if (btType != 1 && btType != 2 && btType != 3)
		{
			//cout << "wrong type" << endl;

			//错误数据通知
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
			sprintf(szLog, "[CTCPClient::recv_head_handler] [包体数据类型=%d]",btType);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}

		//验证包体数据压缩类型
		unsigned char btCompress = m_pchHead[3];
		if (btCompress != 0 && btCompress != 1 && btCompress != 2)
		{
			//cout << "wrong prefix" << endl;

			//错误数据通知
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
			sprintf(szLog, "[CTCPClient::recv_head_handler] [包体数据压缩类型=%d]",btCompress);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}

		//取得包体数据长度
		unsigned long ulLength;
		memcpy(&ulLength, m_pchHead + 4,sizeof(ulLength));

		if (ulLength + 2 > (unsigned long)m_nMaxBodyLength)
		{
#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::recv_body_handler] [增加接收包体缓冲区 增加前[%d] 增加后[%d]]", m_nMaxBodyLength, ulLength + 2);
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
				sprintf(szLog, "[CTCPClient::recv_head_handler] [接收缓存分配失败ClientID=%llu, Error = %s]", (unsigned __int64)this, e.what());
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
				sprintf(szLog, "[CTCPClient::recv_head_handler] [内存分配失败ClientID=%llu]",(unsigned __int64)this);
				CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
				return;

			}
		}

		//异步接收包体
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
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::recv_body_handler] [客户端已经退出]");
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

		//修改最后接收时间
		m_tLastRecv = (long)time(0);

		//检查包尾
		unsigned char btCheckSum;
		unsigned char btSuffix;
		memcpy(&btCheckSum,m_pchBody + ulLength,1);
		memcpy(&btSuffix, m_pchBody + ulLength + 1,1);

		//验证校验和
		unsigned char chCheckSum;
		if (btType == 2)
		{
			chCheckSum = 0;
		} 
		else
		{
			if (bytes_transferred != ulLength + 2)
			{
				//错误数据通知
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
				sprintf(szLog, "[CTCPClient::recv_body_handler] [传输字节数错误 应传输[%d] 已传输[%d]]", ulLength + 2, bytes_transferred);
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
			//错误数据通知
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
			sprintf(szLog, "[CTCPClient::recv_body_handler] [传输前校验和 = %d, 传输后校验和 = %d]", btCheckSum, chCheckSum);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}

		//验证结束标志
		if (btSuffix != 0x16)
		{
			//错误数据通知
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
			sprintf(szLog, "[CTCPClient::recv_body_handler] [结束标志=%d]",btSuffix);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}

		if (btType == 1)	//数据包
		{
			pCommonServer->m_pTCPServerNotifyObj->OnRecvClientData((unsigned __int64)this, m_pchBody, ulLength);
		} 
		else if (btType == 2)	//心跳包
		{
#ifdef __LOG__
			CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::recv_body_handler] [接收心跳]");
#endif
		}
		else if (btType == 3)	//登录验证信息
		{

		}
		else //错误数据
		{
			//错误数据通知
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

		//继续接收下一个包头
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
		//更新最后发送时间
		m_tLastSend = (long)time(0);

		m_listSendData.pop_front();
		if ( !m_listSendData.empty() )
		{
			packet_pt pNextPacket = m_listSendData.front();
			if ( m_ptrSock->is_open() )
			{
#ifdef __LOG__
				char szLog[1024];
				sprintf(szLog, "[CTCPClient::send_mdfdata_handler] [发送队列长度 = %d]", m_listSendData.size());
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
//函数名：  send_nonmdfdata_handler
//作者： yin hui   
//日期： 20110128      
//功能： 发送nonmdf数据的回调函数   
//输入参数：const system::error_code& ec	错误码
//返回值：  无
//修改记录：无
//==================================================================
void CTCPClient::send_nonmdfdata_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec, nonmdfpacket_pt pPacket)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::send_nonmdfdata_handler] [IN]");
#endif

	if (m_bExit)
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::send_nonmdfdata_handler] [客户端已经退出] [OUT]");
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
		//更新最后发送时间
		m_tLastSend = (long)time(0);

		pPacket.reset();

		m_listSendData_NonMdf.pop_front();

		if ( !m_listSendData_NonMdf.empty() )
		{
#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::send_nonmdfdata_handler] [发送队列长度 = %d]", m_listSendData_NonMdf.size());
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
		sprintf(szLog, "[CTCPClient::SendPacket] [发送队列长度 = %d]", m_listSendData_NonMdf.size());
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
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CTCPClient::OnSendMDFData] [客户端已经退出] [OUT]");
#endif
		return;
	}

	if ( m_listSendData.empty() )
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CTCPClient::OnSendMDFData] [发送队列为空，直接发送]");
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
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CTCPClient::OnSendMDFData] [发送队列非空，稍后发送]");
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
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CTCPClient::OnSendOtherData] [客户端已经退出] [OUT]");
#endif
		return;
	}

	if ( m_listSendData_NonMdf.empty() )
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CTCPClient::OnSendOtherData] [发送队列为空，直接发送]");
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
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CTCPClient::OnSendOtherData] [发送队列非空，稍后发送]");
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
			sprintf(szLog, "[CTCPClient::recv_head_handler] [数据堆积,包数[%d]", nSize);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

			return;
		}

		m_listSendData_NonMdf.push_back( pPacket );

//		//更新最后发送时间
//		//m_tLastSend = (long)time(0);
//
//		//m_listSendData_NonMdf.pop_front();
//		if ( !m_listSendData_NonMdf.empty() )
//		{
//#ifdef __LOG__
//			char szLog[1024];
//			sprintf(szLog, "[CTCPClient::OnSendOtherData] [发送队列长度 = %d]", m_listSendData_NonMdf.size());
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
			CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CTCPClient::HB_timer_handler] [客户端已经退出] [OUT]");
#endif
			return;
		}
	}

	//定时器被取消
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
				//非线程方式，不能调用sleep
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
					sprintf(szLog, "[CTCPClient::HB_timer_handler] [心跳包分配失败ClientID=%llu, Error = %s]", (unsigned __int64)this, e.what());
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

// CommonServer.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include "TigerServer.h"

//日志
CLogWriter m_LogWriter;

//==================================================================
//函数名：  CreateMDFTCPServerInstance
//作者： yin hui   
//日期： 20110128     
//功能：  创建CCommonClient实例的接口函数    
//输入参数：ITCPServerNotify* pTCPServerNotifyObj
//返回值：  CCommonClient实例
//修改记录：20110128	取消输入参数boost::asio::io_service& io
//==================================================================
ITCPServer* CreateMDFTCPServerInstance(ITCPServerNotify* pTCPServerNotifyObj )
{
	if ( pTCPServerNotifyObj==NULL )
		return NULL;

#ifdef __LOG__
	if (m_LogWriter.Initialize())
	{
		m_LogWriter.Log(WN_LOG_LEVEL_INFO,"[Initialize log succeed]");
	} 
	else
	{
		//m_LogWriter.Log(WN_LOG_LEVEL_INFO,"Initialize log failed");
		//cout << "初始化日志失败" << endl;
	}
#endif

	return new CTigerServer(pTCPServerNotifyObj );
}

//==================================================================
//函数名： CCommonServer 
//作者： yin hui   
//日期： 20110128     
//功能：   CCommonServer构造函数  
//输入参数：ITCPServerNotify* pTCPServerNotifyObj
//返回值：  无
//修改记录：无
//==================================================================
CTigerServer::CTigerServer(ITCPServerNotify* pTCPServerNotifyObj ): io_service_pool_(5), // m_pIO(new io_service()),
	/*m_acceptor(*m_pIO,ip::tcp::endpoint(ip::tcp::v4(),usPort)),*/m_pTCPServerNotifyObj(pTCPServerNotifyObj)//, m_strand(*m_pIO)
{
	m_acceptor = NULL;
	m_bExit = false;

	m_bAppExit = false;

	m_nThreadPoolSize = 5; //sysconf(_SC_NPROCESSORS_ONLN)

}

//==================================================================
//函数名：  CCommonServer
//作者： yin hui   
//日期： 20110128     
//功能：  CCommonServer析构函数  
//输入参数：无
//返回值：  无
//修改记录：无
//==================================================================
CTigerServer::~CTigerServer(void)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::~CCommonServer] [IN]");
#endif

	if (m_pIO)
	{
		if (m_work)
		{
			m_work.reset();
		}

		delete m_pIO;
		m_pIO = NULL;
	}


#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::~CCommonServer] [OUT]");
#endif

	m_LogWriter.Uninitialize();
}

//==================================================================
//函数名：  Run
//作者： yin hui   
//日期： 20110128      
//功能： 异步消息处理循环线程函数   
//输入参数：无
//返回值：  无
//修改记录：无
//==================================================================
void CTigerServer::Run()
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::Run] [IN]");
#endif

	try
	{
		//if (m_pIO)
		//{
		//	m_pIO->run();
		//}
		io_service_pool_.run();
	}
	catch (std::exception& e)
	{
#ifdef __LOG__
		char szLogMsg[1024];
		sprintf(szLogMsg, "[CCommonServer::Run][Error = %s]",e.what());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLogMsg);
#endif
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::Run] [OUT]");
#endif
}

//释放所有客户端资源
void CTigerServer::DeleteAllClients()
{
	boost::mutex::scoped_lock lock(m_mapClient_mutex);

	std::map<unsigned __int64, boost::shared_ptr<CTCPClient> >::iterator itr;
	while ( ! m_mapClient.empty())
	{
		itr = m_mapClient.begin();
		//boost::shared_ptr<CTCPClient> pClient = (*itr).second;
		//pClient->CloseSocket();

		m_pTCPServerNotifyObj->OnClientDisconnect((*itr).first);

		//delete pClient;
		//pClient = NULL;

		m_mapClient.erase(itr);
	}
}

void CTigerServer::BeginRunThread()
{
	for (int i = 0; i < m_nThreadPoolSize; i ++)
	{
		m_ThreadGroup.create_thread((boost::bind(&CTigerServer::Run,this)));
	}
	
	m_ThreadGroup.join_all();

	if (m_acceptor)
	{
		m_acceptor->close();
		delete m_acceptor;
		m_acceptor = NULL;
	}

	DeleteAllClients();

	if (m_pIO)
	{
		m_pIO->reset();
		//m_pIO.reset();
		delete m_pIO;
		m_pIO = NULL;
	}

	m_bExit = true;

	if ( m_bAppExit )
	{
		if (m_pTCPServerNotifyObj)
		{
			m_pTCPServerNotifyObj->OnClientDisconnect(0);
		}
	}
}

//==================================================================
//函数名： MDFServerListen 
//作者： yin hui   
//日期： 20110128      
//功能： 在指定端口启动TCP服务器侦听   
//输入参数：unsigned short usPort	侦听端口
//			bool bAuth				是否需要验证
//返回值：  true/false
//修改记录：无
//==================================================================
bool CTigerServer::MDFServerListen( unsigned short usPort,bool bAuth )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::MDFServerListen] [IN]");
#endif

	m_pIO = &(io_service_pool_.get_io_service()); //new boost::asio::io_service();
	m_work = boost::shared_ptr<boost::asio::io_service::work>(new boost::asio::io_service::work(*m_pIO));
	m_strand = new boost::asio::strand(*m_pIO);

	try
	{
		m_acceptor = new boost::asio::ip::tcp::acceptor(*m_pIO,boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),usPort),true);
	}
	catch (.../*boost::exception& e*/)
	{
		if (m_pIO)
		{
			if (m_work)
			{
				m_work.reset();
			}

			delete m_pIO;
			m_pIO = NULL;
		}

#ifdef __LOG__
		char szLogMsg[1024];
		sprintf(szLogMsg, "[CCommonServer::MDFServerListen][Listen port %d is used]",usPort);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLogMsg);
#endif
		return false;
	}

	sock_pt sock(new boost::asio::ip::tcp::socket(io_service_pool_.get_io_service()));
	//ip::tcp::socket* sock = new ip::tcp::socket(*m_pIO);

	m_acceptor->async_accept(*sock, boost::bind(&CTigerServer::accept_handler, this, boost::asio::placeholders::error, sock, true));

	m_bAuth = bAuth;

	//boost::thread t(&CTigerServer::BeginRunThread, this);
	boost::thread t(&CTigerServer::Run, this);

	//m_ThreadGroup.create_thread((bind(&CCommonServer::BeginRunThread,this)));


#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::MDFServerListen] [OUT]");
#endif

	return true;
}

//==================================================================
//函数名： OtherServerListen 
//作者： yin hui   
//日期： 20110128      
//功能： 在指定端口启动非MDF协议的TCP服务器侦听（无心跳包）
//输入参数：unsigned short usPort	侦听端口
//			bool bAuth				是否需要验证
//返回值：  true/false
//修改记录：无
bool CTigerServer::OtherServerListen( unsigned short usPort,bool bAuth )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::OtherServerListen] [IN]");
#endif

	m_pIO = &(io_service_pool_.get_io_service()); //new boost::asio::io_service();
	m_work = boost::shared_ptr<boost::asio::io_service::work>(new boost::asio::io_service::work(*m_pIO));
	m_strand = new boost::asio::strand(*m_pIO);

	try
	{
		m_acceptor = new boost::asio::ip::tcp::acceptor(*m_pIO, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),usPort),true);
	}
	catch (.../*boost::exception& e*/)
	{
		if (m_pIO)
		{
			if (m_work)
			{
				m_work.reset();
			}

			delete m_pIO;
			m_pIO = NULL;
		}

#ifdef __LOG__
		char szLogMsg[1024];
		sprintf(szLogMsg, "[CCommonServer::OtherServerListen] [Listen port %d is used]",usPort);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLogMsg);
#endif
		return false;
	}

	sock_pt sock(new boost::asio::ip::tcp::socket(io_service_pool_.get_io_service()));
	//ip::tcp::socket* sock = new ip::tcp::socket(*m_pIO);
	m_acceptor->async_accept(*sock,  boost::bind(&CTigerServer::accept_handler, this, boost::asio::placeholders::error, sock, false));

	m_bAuth = bAuth;

	boost::thread t(&CTigerServer::Run, this);
	//boost::thread t(&CTigerServer::BeginRunThread,this);

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::OtherServerListen] [OUT]");
#endif

	return true;
}


//==================================================================
//函数名：  accept_handler
//作者： yin hui   
//日期： 20110128     
//功能：  接受客户端连接的回调函数  
//输入参数：const system::error_code& ec	错误码
//			sock_pt sock					用于与客户端数据传输的socket
//返回值：无  
//修改记录：无
//==================================================================
void CTigerServer::accept_handler(const boost::system::error_code& ec, boost::shared_ptr<boost::asio::ip::tcp::socket> sock, bool bMDF)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::accept_handler] [IN]");
#endif

	std::string str = ec.message();
	if (ec)
	{
#ifdef __LOG__
		char szLog[1024];
		std::string str = ec.message();
		sprintf(szLog, "[CCommonServer::accept_handler] [%s]",str.c_str());
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		server_disconnect_handler(-1,sock,E_WN_SERVER_ACCEPT_ACCEPT_FAIL);
		return;
	}

	std::string client_ip;
	unsigned short usPort;
	client_ip = sock->remote_endpoint().address().to_string();
	usPort = sock->remote_endpoint().port();

	sock->set_option(boost::asio::ip::tcp::no_delay(true)); 

	if (m_bAuth)
	{
	} 
	else
	{
		//CTCPClient* pClient = new CTCPClient(*m_pIO,sock,this);
		
		try
		{
			boost::shared_ptr<CTCPClient> pClient( new CTCPClient(sock->get_io_service(), sock, this, m_strand) );

			{
				//cout << "accept_handler" << endl;
				boost::mutex::scoped_lock lock(m_mapClient_mutex);
				std::map<unsigned __int64, boost::shared_ptr<CTCPClient> >::iterator it=m_mapClient.find( (unsigned __int64)&*pClient );
				if ( it!=m_mapClient.end() )
				{
					std::cout << "SDK异常:重复ID" << std::endl;
					//continue accept
					sock_pt nextsock(new boost::asio::ip::tcp::socket(io_service_pool_.get_io_service()));
					m_acceptor->async_accept(*nextsock, boost::bind(&CTigerServer::accept_handler,this, boost::asio::placeholders::error,nextsock,bMDF)) ;
					return;
				}
				else
				{
					m_mapClient.insert(std::pair<unsigned __int64, boost::shared_ptr<CTCPClient> >((unsigned __int64)&*pClient/*pClient.get()*/,pClient));
				}
			}

			m_pIO->post( bind(&CTCPClient::Start, pClient, sock, bMDF));

			m_pTCPServerNotifyObj->OnAccept( (unsigned __int64)&*pClient/*pClient.get()*/, client_ip.c_str(), usPort);

			sock_pt nextsock(new boost::asio::ip::tcp::socket(io_service_pool_.get_io_service()));
			m_acceptor->async_accept(*nextsock, boost::bind(&CTigerServer::accept_handler, this, boost::asio::placeholders::error, nextsock, bMDF) );

		}
		catch (std::bad_alloc& e)
		{
#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::RecvData] [CTCPClient对象分配失败, Error = %s]", e.what());
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;
		}
	
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::accept_handler] [OUT]");
#endif
}

//==================================================================
//函数名：  SendData
//作者： yin hui   
//日期： 20110128     
//功能： 发送数据给指定的客户端   
//输入参数：unsigned int unClientID		客户端ID
//			unsigned char * pucData,				数据缓冲
//			unsigned int unDataSize	数据长度
//返回值：  TRUE/FALSE
//修改记录：无
//==================================================================
bool CTigerServer::SendDataToMDFClient( unsigned __int64 unClientID, const unsigned char * pucData, unsigned int unDataSize )
{
#ifdef __LOG__
	char szLog[1024];
	sprintf(szLog, "[CCommonServer::SendDataToMDFClient] [IN] [ClientID = %llu]", unClientID);
	CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

	//CTCPClient* pClient = NULL;
	boost::shared_ptr<CTCPClient> pClient;
	{
		//cout << "SendDataToMDFClient" << endl;
		boost::mutex::scoped_lock lock(m_mapClient_mutex);
		std::map<unsigned __int64, boost::shared_ptr<CTCPClient> >::iterator it = m_mapClient.find( unClientID );
		if ( it==m_mapClient.end() )
		{
#ifdef __LOG__
			CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::SendDataToMDFClient] [OUT] [找不到ClientID]");
#endif
			return false;
		}

		pClient = it->second;

		if ( pClient && pClient->m_bExit )
		{
#ifdef __LOG__
			CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::SendDataToMDFClient] [OUT] [客户端已经退出或者删除]");
#endif
			return false;
		}

		if (pClient)
		{
			packet_pt pPacket(new CPacket(1,0,pucData,unDataSize));
			m_pIO->post( bind(&CTCPClient::OnSendMDFData, pClient->shared_from_this(), pClient->m_ptrSock, pPacket));
		}
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::SendDataToMDFClient] [OUT]");
#endif

	return true;
}

//==================================================================
//函数名：  SendDataToOtherClient
//作者： yin hui   
//日期： 20110128     
//功能：  发送数据给非MDF客户端（与OtherServerListen配对使用）
//输入参数：unsigned int unClientID     客户ID   
//			unsigned char * pucData                数据缓冲
//			unsigned int unDataSize    数据长度
//返回值：无  
//修改记录：无
//==================================================================
bool CTigerServer::SendDataToOtherClient( unsigned __int64 unClientID, const unsigned char * pucData, unsigned int unDataSize )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::SendDataToOtherClient] [IN]");
#endif

	boost::shared_ptr<CTCPClient> pClient;
	{
		boost::mutex::scoped_lock lock(m_mapClient_mutex);
		pClient = m_mapClient[unClientID];

		if (pClient)
		{
			nonmdfpacket_pt pPacket(new CNonMDFPacket(pucData,unDataSize));
			m_pIO->post(boost::bind(&CTCPClient::OnSendOtherData, pClient->shared_from_this(), pClient->m_ptrSock, pPacket));
		}
	}
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::SendDataToOtherClient] [OUT]");
#endif

	return true;
}


//断开所有客户端连接
void CTigerServer::CloseAllClients()
{
	boost::mutex::scoped_lock lock(m_mapClient_mutex);

	std::map<unsigned __int64, boost::shared_ptr<CTCPClient> >::iterator itr;
	for ( itr = m_mapClient.begin(); itr != m_mapClient.end(); itr ++)
	{
		boost::shared_ptr<CTCPClient> pClient = (*itr).second;
		if ( pClient.get() )
		{
			pClient->CloseSocket();
		}

		m_pTCPServerNotifyObj->OnClientDisconnect((*itr).first);
	}
}

void CTigerServer::do_close()
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::do_close] [IN]");
#endif

	CloseAllClients();

	if ( m_pIO )
	{
		m_pIO->stop();
	}

	{
		m_work.reset();
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::do_close] [OUT]\n");
#endif
}

//==================================================================
//函数名：  CloseSocket
//作者： yin hui   
//日期： 20110128      
//功能：  断开所有客户端连接  
//输入参数：无
//返回值：  无
//修改记录：无
//==================================================================
//关闭TCP Server
void CTigerServer::CloseSocket()
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::CloseSocket] [IN]");
#endif

	m_bAppExit = true;

	if ( m_pIO )
	{
		m_pIO->post(boost::bind(&CTigerServer::do_close, this));
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::CloseSocket] [OUT]");
#endif
		return;
	}
	else     //run()已经退出，不能通过post退出
	{
		CloseAllClients();

		if ( m_pTCPServerNotifyObj )
		{
			m_pTCPServerNotifyObj->OnClientDisconnect(0);
		}
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::CloseSocket] [OUT]");
#endif
}

//==================================================================
//函数名：  disconnect_handler
//作者： yin hui   
//日期： 20110128     
//功能：  错误处理  
//输入参数：int nErrorCode 错误码
//返回值：  无
//修改记录：无
//==================================================================
void CTigerServer::server_disconnect_handler(unsigned __int64 unClientID,boost::shared_ptr<boost::asio::ip::tcp::socket> sock,int nErrorCode)
{
#ifdef __LOG__
	char szLog[1024];
	sprintf(szLog, "[CCommonServer::server_disconnect_handler] [IN] [nErrorCode = %d]", nErrorCode);
	CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

	//CTCPClient* pClient = NULL;

	switch (nErrorCode)
	{
	case E_WN_SERVER_ACCEPT_ADDR_USED:
	case E_WN_SERVER_ACCEPT_ACCEPT_FAIL:
	case E_WN_SERVER_ACCEPT_SENDLOGIN_FAIL:
	case E_WN_SERVER_ACCEPT_RECVLOGIN_FAIL:
		break;

	case E_WN_SERVER_SEND_DATA_FAIL:
	case E_WN_SERVER_SEND_HB_FAIL:
	case E_WN_SERVER_RECV_HEAD_FAIL:
	case E_WN_SERVER_RECV_BODY_FAIL:
	case E_WN_SERVER_RECV_TIMEOUT:
	case E_WN_SERVER_RECV_NOMDF_FAIL:
	case E_WN_SERVER_RECV_NO_HB:
		m_pTCPServerNotifyObj->OnClientDisconnect(unClientID);
		
		if ( m_pIO )
		{
			{
				boost::mutex::scoped_lock lock(m_mapClient_mutex);

				std::map<unsigned __int64, boost::shared_ptr<CTCPClient> >::iterator it = m_mapClient.find( unClientID );
				if ( it!=m_mapClient.end() )
				{
					m_mapClient.erase( it );
				}
			}
		}

		break;

	case E_WN_SERVER_NOTIFY_CLOSE:
		break;

	default :
		break;
	}
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonServer::server_disconnect_handler] [OUT]");
#endif
}

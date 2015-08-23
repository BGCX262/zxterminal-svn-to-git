// CommonClient.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include "Connection.h"

#include "TigerClient.h"

//==================================================================
//函数名： CConnection 
//作者： yin hui   
//日期： 20110128  
//功能： CConnection构造函数，初始化类   
//输入参数：ITCPClientNotify* pTCPClientNotifyObj
//返回值：  无
//修改记录：取消输入参数boost::asio::io_service& io
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
 //函数名：~CConnection  
 //作者： yin hui   
 //日期： 20110128   
 //功能： CConnection析构函数，释放资源，关闭socket   
 //输入参数：
 //返回值：  
 //修改记录：
 //==================================================================
CConnection::~CConnection(void)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::~CConnection] [IN]");
#endif

	//必须先析构与io_service相关的对象，才能最后析构io_service
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
//函数名： Run 
//作者： yin hui   
//日期： 20110128    
//功能： 进入异步消息处理循环，如果退出，则修改标志，以便退出心跳线程和数据接收   
//输入参数：无
//返回值： 无 
//修改记录：20110221 Run()在线程中修改m_socket（disconnect_handler中修改），导致接收数据出错，取消调用disconnect_handler
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

//20105016 把错误通知放到所有的Run退出后才执行
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
//函数名： ConnectMDFServer 
//作者： yin hui   
//日期： 20110128     
//功能： 连接指定的TCP服务器  
//输入参数：const char *szIP          IP地址 
//			unsigned short usPort     端口
//			bool bAuth                是否需要验证
//返回值：  无
//修改记录：
//			20110128   由于异步消息处理循环m_pIO->run()是阻塞的，因此创建一个线程运行   
//			20110218   增加keep_alive选项
//==================================================================
void CConnection::ConnectMDFServer( const char *szIP, unsigned short usPort,bool bAuth, int nTimeout)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::ConnectMDFServer] [IN]");
#endif

	m_bConnect = false;

	//验证IP地址
	//验证端口

	m_pIO = boost::shared_ptr<boost::asio::io_service>(new boost::asio::io_service());
	m_strand = new boost::asio::strand(*m_pIO);

	//m_ReadTimer =  make_shared<deadline_timer>(ref(*m_pIO));
	m_ConnectTimer	= boost::make_shared<boost::asio::deadline_timer>(boost::ref(*m_pIO));
	m_HBTimer		= boost::make_shared<boost::asio::deadline_timer>(boost::ref(*m_pIO));
	m_HBCheckTimer	= boost::make_shared<boost::asio::deadline_timer>(boost::ref(*m_pIO));
	m_socket		= sock_pt(new boost::asio::ip::tcp::socket(*m_pIO));

	//异步连接服务器

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

	//设置连接超时定时器
	m_ConnectTimer->expires_from_now(boost::posix_time::seconds(nTimeout));
	m_ConnectTimer->async_wait(m_strand->wrap(bind(&CConnection::connect_timer_handler, shared_from_this(), m_socket, boost::asio::placeholders::error)));

	//创建异步消息处理循环线程
	boost::thread t(&CConnection::BeginRunThread, shared_from_this());

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::ConnectMDFServer] [OUT]\n");
#endif
}

//==================================================================
//函数名： ConnectMDFServer 
//作者： yin hui   
//日期： 20110128     
//功能： 连接非MDF协议的TCP服务器
//输入参数：const char *szIP          IP地址 
//			unsigned short usPort     端口
//			bool bAuth                是否需要验证
//返回值：  无
//修改记录：
//			20110128   由于异步消息处理循环m_pIO->run()是阻塞的，因此创建一个线程运行   
//			20110218   增加keep_alive选项
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

	//异步连接服务器

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

	//设置连接超时定时器
	m_ConnectTimer->expires_from_now(boost::posix_time::seconds(nTimeout));
	m_ConnectTimer->async_wait(m_strand->wrap(boost::bind(&CConnection::connect_timer_handler, shared_from_this(), m_socket, boost::asio::placeholders::error)));

	//创建异步消息处理循环线程
	//boost::thread t(&CConnection::Run,this);
	boost::thread t(&CConnection::BeginRunThread, shared_from_this());

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::ConnectOtherServer] [OUT]\n");
#endif
}

//发送数据给非MDF协议的服务器
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
//函数名：  connect_timer_handler
//作者： yin hui   
//日期： 20110128     
//功能： 连接超时回调函数   
//输入参数：const system::error_code& ec 错误码
//返回值：  无
//修改记录：无
//==================================================================
void CConnection::connect_timer_handler(sock_pt sock, const boost::system::error_code& ec)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::connect_timer_handler] [IN]");
#endif

	if( ec ) //timer已经取消
	{
		//std::cout << "timer is cancelled." << endl;
	}
	else  //timer已经超时
	{
		disconnect_handler(sock,E_WN_CLIENT_CONNECT_TIMEOUT);
		m_nErrorCode = E_WN_CLIENT_CONNECT_TIMEOUT;
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::connect_timer_handler] [OUT]\n");
#endif
}

//==================================================================
//函数名： connect_handler 
//作者： yin hui   
//日期： 20110128     
//功能：  异步连接回调函数  
//输入参数：const system::error_code& ec,	错误吗
//			sock_pt sock,					用于连接的socket
//			const char *szIP,				IP地址
//			unsigned short usPort,			端口
//			bool bAuth						是否需要验证
//返回值：  无
//修改记录：20100314 增加disconnect_handler关闭socket，避免第一次连接不上第二次连接程序僵死的问题（io_service没退出）
//==================================================================
void CConnection::connect_handler(sock_pt sock, const boost::system::error_code& ec, const char *szIP, unsigned short usPort, bool bAuth)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::connect_handler] [IN]");
#endif

	//取消timer
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

	//启动心跳检测
	m_HBCheckTimer->expires_from_now(boost::posix_time::seconds(3 * HB_SEND_PERIOD));
	m_HBCheckTimer->async_wait(m_strand->wrap(boost::bind(&CConnection::check_hb_timer_handler, shared_from_this(), sock, boost::asio::placeholders::error)));

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::connect_handler] [OUT]\n");
#endif
}


//==================================================================
//函数名：  check_hb_timer_handler
//作者： yin hui   
//日期： 20110211      
//功能：  检测心跳定时器的回调函数
//输入参数：const system::error_code& ec    错误码
//返回值：  无
//修改记录：无
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

	//定时器被取消
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
	else //超过45s没有接收到数据
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,"[CConnection::check_hb_timer_handler] [超过45秒没有接收到数据]");
#endif

		disconnect_handler(sock,E_WN_CLIENT_RECV_NO_HB);
		m_nErrorCode = E_WN_CLIENT_RECV_NO_HB;
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::check_hb_timer_handler] [OUT]\n");
#endif
}


//==================================================================
//函数名： connect_NOT_MDF_handler 
//作者： yin hui   
//日期： 20110128     
//功能：  异步连接回调函数  
//输入参数：const system::error_code& ec,	错误吗
//			sock_pt sock,					用于连接的socket
//			const char *szIP,				IP地址
//			unsigned short usPort,			端口
//			bool bAuth						是否需要验证
//返回值：  无
//修改记录：20100314 增加disconnect_handler关闭socket，避免第一次连接不上第二次连接程序僵死的问题（io_service没退出）
//==================================================================
void CConnection::connect_NOT_MDF_handler(sock_pt sock, const boost::system::error_code& ec, const char *szIP, unsigned short usPort,bool bAuth)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::connect_NOT_MDF_handler] [IN]");
#endif

	//取消timer
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
//函数名：  SendDataToMDFServer
//作者： yin hui   
//日期： 20110128     
//功能： 发送数据给服务器   
//输入参数：unsigned char * pData,				数据缓冲
//			unsigned int ulDataSize	数据长度
//返回值：  TRUE/FALSE
//修改记录：20110215 修改CPacket的析构函数，并且改为shared_ptr，保证每次资源释放，避免内存泄露
//          20110506 在回调函数send_data_handler中增加packet_pt pPacket参数，避免出现异步发送还没有完成
//                   就释放缓冲，导致出现“系统检测到在一个调用中尝试使用指针参数时的无效指针地址”的问题
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
//函数名：  send_data_handler
//作者： yin hui   
//日期： 20110128    
//功能：发送数据的回调函数
//输入参数：onst system::error_code& ec 错误码
//返回值：  无
//修改记录：无
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
		sprintf(szLog, "[CTCPClient::send_data_handler] 应发送[%d] 已发送[%d]", unDataLen, bytes_transferred);
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
//函数名：  send_mdfdata_handler
//作者： yin hui   
//日期： 20110602    
//功能：发送MDF数据的回调函数
//输入参数：onst system::error_code& ec 错误码
//返回值：  无
//修改记录：无
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
		sprintf(szLog, "[CConnection::send_mdfdata_handler] 应发送[%d] 已发送[%d]", unDataLen, bytes_transferred);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

		disconnect_handler(sock,E_WN_CLIENT_SEND_DATA_FAIL);
		m_nErrorCode = E_WN_CLIENT_SEND_DATA_FAIL;
	}
	else
	{
		//更新最后发送时间
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
//函数名：  send_nonmdfdata_handler
//作者： yin hui   
//日期： 20110128    
//功能：发送数据的回调函数
//输入参数：onst system::error_code& ec 错误码
//返回值：  无
//修改记录：无
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
		sprintf(szLog, "[CConnection::send_nonmdfdata_handler] 应发送[%d] 已发送[%d]", unDataLen, bytes_transferred);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif

		disconnect_handler(sock,E_WN_CLIENT_SEND_DATA_FAIL);
		m_nErrorCode = E_WN_CLIENT_SEND_DATA_FAIL;
	}
	else
	{
		//更新最后发送时间
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
//函数名： send_hb_handler 
//作者： yin hui   
//日期： 20110128    
//功能： 发送心跳包的回调函数   
//输入参数：const system::error_code& ec 错误码
//返回值：  无
//修改记录：无
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
		sprintf(szLog, "[CTCPClient::send_hb_handler] 应发送[%d] 已发送[%d]", unDataLen, bytes_transferred);
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
//函数名： RecvData 
//作者： yin hui   
//日期： 20110128    
//功能： 接收数据循环   
//输入参数：无
//返回值：  无
//修改记录：无
//==================================================================
void CConnection::RecvData( sock_pt sock )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::RecvData] [IN]");
#endif

	if ( !m_socket || !sock || !m_strand )
		return;

	//异步接收包头
	//boost::shared_ptr<unsigned char> btHead(new unsigned char[8], deleter);
	async_read(*sock, boost::asio::buffer(m_pchHead, 8),  m_strand->wrap(boost::bind(&CConnection::recv_head_handler,shared_from_this(),
		sock, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred/*, btHead*/)));

	//设置接收包头超时定时器
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
//函数名：  read_timer_handler
//作者： yin hui   
//日期： 20110128    
//功能： 接收包头超时回调函数   
//输入参数：const system::error_code& ec 错误码
//返回值：  无
//修改记录：20110128  超时不关闭socket，继续接收
//==================================================================
/*void CConnection::read_timer_handler( sock_pt sock, const system::error_code& ec)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::read_timer_handler] [IN]");
#endif

	if(ec) //timer已经取消
	{
		//std::cout << "timer is canceled." << endl;
	}
	else  //timer已经超时
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
//函数名：  HB_timer_handler
//作者： yin hui   
//日期： 20110128    
//功能： 发送心跳定时器函数   
//输入参数：const system::error_code& ec 错误码
//返回值：  无
//修改记录：
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
//函数名：  recv_head_handler
//作者： yin hui   
//日期： 20110128    
//功能： 接收包头的回调函数  
//输入参数：const system::error_code& ec 错误码
//          shared_ptr<unsigned char> btHead      接收包头的缓冲
//返回值：  无
//修改记录：无
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

	//取消接收超时timer
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

	//验证包头长度
	if (bytes_transferred != 8)
	{
		//错误数据通知
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
		}

		disconnect_handler(sock,E_WN_CLIENT_RECV_HEAD_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_HEAD_FAIL;
#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_head_handler] [传输字节数错误 应传输[%d] 已传输[%d] = %d]", 8 ,bytes_transferred);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		return;
	}

	//验证包头

	//验证包头开始标志
	unsigned char btBegin = m_pchHead[0];
	if (btBegin != 0x55)
	{
		//错误数据通知
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
		}

		disconnect_handler(sock,E_WN_CLIENT_RECV_HEAD_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_HEAD_FAIL;
#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_head_handler] [开始标志 = %d]",btBegin);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		return;
	}

	//验证包头协议版本
	unsigned char btVersion = m_pchHead[1];
	if (btVersion != 1)
	{
		//错误数据通知
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
		}

		disconnect_handler(sock,E_WN_CLIENT_RECV_HEAD_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_HEAD_FAIL;
#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_head_handler] [协议版本 = %d]", btVersion);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		return;
	}

	//验证包体数据类型
	unsigned char btType = m_pchHead[2];
	if (btType != 1 && btType != 2 && btType != 3)
	{
		//错误数据通知
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
		}

		disconnect_handler(sock,E_WN_CLIENT_RECV_HEAD_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_HEAD_FAIL;

#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_head_handler] [包体数据类型 = %d]", btType);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		return;
	}

	//验证包体数据压缩类型
	unsigned char btCompress = m_pchHead[3];
	if (btCompress != 0 && btCompress != 1 && btCompress != 2)
	{
		//错误数据通知
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
		}

		disconnect_handler(sock,E_WN_CLIENT_RECV_HEAD_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_HEAD_FAIL;
#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_head_handler] [包体数据压缩类型 = %d]", btCompress);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
		return;
	}

	//取得包体数据长度
	unsigned int ulLength;
	memcpy(&ulLength, m_pchHead + 4, sizeof(ulLength));

	if (ulLength + 2 > (unsigned long)m_nMaxBodyLength)
	{
#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_head_handler] [增加接收包体缓冲区 增加前[%d] 增加后[%d]]", m_nMaxBodyLength, ulLength + 2);
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
			sprintf(szLog, "[CConnection::recv_head_handler] [接收缓存分配失败ClientID=%llu, Error = %s]", (unsigned __int64)this, e.what());
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
			sprintf(szLog, "[CConnection::recv_head_handler] [内存分配失败ClientID=%llu]",(unsigned __int64)this);
			CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
#endif
			return;

		}
	}

	//继续接收
	if ( sock )
	{
		//异步接收包体
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

	//数据包通知
	if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
	{
		//pCommonClient->m_pTCPClientNotifyObj->OnRecvServerData( &*btBuffer, bytes_transferred );
		pCommonClient->m_pTCPClientNotifyObj->OnRecvServerData( m_pchBody, bytes_transferred );
	}

	//继续接收数据
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
//函数名：  recv_body_handler
//作者： yin hui   
//日期： 20110128    
//功能：  接收包体的回调函数  
//输入参数：const system::error_code& ec,	错误码
//			unsigned char btType,					包体数据类型
//			shared_ptr<unsigned char> btBody,		包体数据
//			unsigned int ulLength					包体数据长度
//返回值： 无 
//修改记录：无
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

	//修改最后接收时间
	m_tLastRecv = (long)time(0);

	//检查包尾
	unsigned char btCheckSum;
	unsigned char btSuffix;
	memcpy(&btCheckSum, m_pchBody + ulLength, 1);
	memcpy(&btSuffix, m_pchBody + ulLength + 1, 1);

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
			if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
			{
				pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
			}

			disconnect_handler(sock,E_WN_CLIENT_RECV_BODY_FAIL);
			m_nErrorCode = E_WN_CLIENT_RECV_BODY_FAIL;

#ifdef __LOG__
			char szLog[1024];
			sprintf(szLog, "[CTCPClient::recv_body_handler] [传输字节数错误 应传输[%d] 已传输[%d]]", ulLength + 2, bytes_transferred);
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
		//错误数据通知
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();		
		}

		disconnect_handler(sock,E_WN_CLIENT_RECV_BODY_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_BODY_FAIL;

#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_body_handler] [传输前校验和 = %d, 传输后校验和 = %d, 长度 = %d]", 
			btCheckSum, chCheckSum, ulLength);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_body_handler] [OUT]\n");
#endif
		return;
	}

	//验证结束标志
	if (btSuffix != 0x16)
	{
		//错误数据通知
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
		}

		disconnect_handler(sock,E_WN_CLIENT_RECV_BODY_FAIL);
		m_nErrorCode = E_WN_CLIENT_RECV_BODY_FAIL;

#ifdef __LOG__
		char szLog[1024];
		sprintf(szLog, "[CConnection::recv_body_handler] [结束标志=%d]",btSuffix);
		CLogWriter::Log(WN_LOG_LEVEL_ERROR,szLog);
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_body_handler] [OUT]\n");
#endif
		return;
	}

	if (btType == 1)  //数据包
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_body_handler] [OnrecvServerData begin]\n");
#endif
		//数据包通知
		//if (!m_bAppExit)//因为CloseSocket退出时要等待，而且此时要通知数据到达，对于有MFC的程序，此时会出现界面僵死，因为此时上一个消息没处理完
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnRecvServerData(m_pchBody,ulLength);
		}
	} 
	else if (btType == 2)	//心跳包
	{
#ifdef __LOG__
		CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_body_handler] [接收心跳]");
#endif
	}
	else if (btType == 3)	//登录验证信息
	{

	}
	else //错误数据
	{
		//错误数据通知
		//if (!m_bAppExit)//因为CloseSocket退出时要等待，而且此时要通知错误数据，对于有MFC的程序，此时会出现界面僵死，因为此时上一个消息没处理完
		if (pCommonClient && pCommonClient->m_pTCPClientNotifyObj)
		{
			pCommonClient->m_pTCPClientNotifyObj->OnServerDataError();
		}
	}
	
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_body_handler] [recv next]\n");
#endif

	//继续接收下一个包头
	//boost::shared_ptr<unsigned char> btHead(new unsigned char[8], deleter);
	async_read(*sock, boost::asio::buffer(m_pchHead, 8),  m_strand->wrap(boost::bind(&CConnection::recv_head_handler, shared_from_this() ,
		sock, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred/*, btHead*/)));

	//btBody.reset();

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CConnection::recv_body_handler] [OUT]\n");
#endif
}

//==================================================================
//函数名： do_close 
//作者： yin hui   
//日期： 20110223     
//功能： 关闭socket  
//输入参数：无
//返回值：  无
//修改记录：无
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
//函数名： CloseSocket 
//作者： yin hui   
//日期： 20110128     
//功能： 与服务器断开网络连接，退出心跳和数据接收，关闭socket  
//输入参数：无
//返回值：  无
//修改记录：无
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

	if (m_bConnect) //已经连接上
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
	else //没有连接上
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
//函数名：  disconnect_handler
//作者： yin hui   
//日期： 20110128     
//功能：  错误处理  
//输入参数：int nErrorCode 错误码
//返回值：  无
//修改记录：无
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

	//程序退出时，先不关闭socket，在析构函数中关闭
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

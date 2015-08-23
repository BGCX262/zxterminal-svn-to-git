#ifndef _ITCP_INCLUDE_20110124_
#define _ITCP_INCLUDE_20110124_

#pragma once

#include "TCPClient.h"
#include "LogWriter.h" 
#include "io_service_pool.hpp"

#define HEADER_LENGTH        8
#define MAX_BODY_LENGTH      1024 * 1024

enum E_WN_SERVER_ERROR_CODE
{
	E_WN_SERVER_ACCEPT_OK				= 0,	//接收连接成功
	E_WN_SERVER_ACCEPT_ADDR_USED        = -1,
	E_WN_SERVER_ACCEPT_ACCEPT_FAIL		= -2,	//接收连接失败
	E_WN_SERVER_ACCEPT_SENDLOGIN_FAIL	= -3,	//发送登录验证确认信息失败
	E_WN_SERVER_ACCEPT_RECVLOGIN_FAIL	= -4,	//接收登陆验证信息失败
	
	E_WN_SERVER_SEND_DATA_FAIL			= -5,	//发送数据失败
	E_WN_SERVER_SEND_HB_FAIL			= -6,	//发送心跳失败
	E_WN_SERVER_RECV_HEAD_FAIL			= -7,	//接收包头失败
	E_WN_SERVER_RECV_BODY_FAIL			= -8,	//接收包体失败
	E_WN_SERVER_RECV_TIMEOUT			= -9,	//接收超时
	E_WN_SERVER_RECV_NOMDF_FAIL			= -10,	//接收非MDF数据失败
	E_WN_SERVER_RECV_NO_HB				= -11,	//长时间没有接收到心跳
	E_WN_SERVER_NOTIFY_CLOSE			= -12,	//外部通知断开连接
	E_WN_SERVER_TOO_MUCH_DATA			= -13	//数据堆积
};

class CTigerServer : public ITCPServer
{
public:
	CTigerServer(ITCPServerNotify* pTCPServerNotifyObj);
	~CTigerServer(void);

public:
	//在指定端口启动TCP服务器侦听
	virtual bool MDFServerListen( unsigned short usPort,bool bAuth );

	//发送数据给指定的客户端
	virtual bool SendDataToMDFClient( unsigned __int64 unClientID, const unsigned char * pucData, unsigned int unDataSize );

	//在指定端口启动非MDF协议的TCP服务器侦听（无心跳包）
	virtual bool OtherServerListen( unsigned short usPort,bool bAuth=false );

	//发送数据给非MDF客户端（与OtherServerListen配对使用）
	virtual bool SendDataToOtherClient( unsigned __int64 unClientID, const unsigned char * pucData, unsigned int unDataSize );

	//关闭TCP Server
	virtual void CloseSocket();

public:
	//接受客户端连接的回调函数
	void accept_handler(const boost::system::error_code& ec, boost::shared_ptr<boost::asio::ip::tcp::socket> sock, bool bMDF=true);

	//异步消息处理循环线程函数
	void Run();

	//启动io_service线程组的线程函数
	void BeginRunThread();

	void do_close();

	void CloseAllClients();

	void DeleteAllClients();

public:
	//错误处理
	void server_disconnect_handler(unsigned __int64 unClientID, boost::shared_ptr<boost::asio::ip::tcp::socket> sock, int nErrorCode);

public:
	//通知对象
	ITCPServerNotify* m_pTCPServerNotifyObj;

public:
	//服务端的io_service
	//shared_ptr<io_service> m_pIO;
	boost::asio::io_service* m_pIO;

	//服务端acceptor
	boost::asio::ip::tcp::acceptor* m_acceptor;

	//客户端容器
	std::map<unsigned __int64, boost::shared_ptr<CTCPClient> > m_mapClient;

	//客户端容器的互斥量
	boost::mutex m_mapClient_mutex;

	//验证标志
	bool m_bAuth;

	//心跳间隔
	int m_nHBInterval;

	//最后接收时间
	long m_tLastRecv;

	//最后发送时间
	long m_tLastSend;

	//退出标志
	bool m_bExit;
	
	//退出APP标志
	bool m_bAppExit;	

    //io_service线程池线程数目
	int m_nThreadPoolSize;

	//io_service线程组
	boost::thread_group m_ThreadGroup;

	boost::asio::strand* m_strand;

	boost::shared_ptr<boost::asio::io_service::work> m_work;

	/// The pool of io_service objects used to perform asynchronous operations.
	io_service_pool io_service_pool_;

};

#endif





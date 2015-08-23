#ifndef _CONN_INCLUDE_20110617_
#define _CONN_INCLUDE_20110617_

#include "Common.h"
#include "ITigerNet.h"

#ifdef __LOG__
#include "LogWriter.h" 
#endif

#define HEADER_LENGTH        8
#define MAX_BODY_LENGTH      1024 * 1024

//心跳发送间隔
#define HB_SEND_PERIOD            15

enum E_WN_CLIENT_ERROR_CODE
{
	E_WN_CLIENT_CONNECT_OK          = 0,	//连接超时
	E_WN_CLIENT_CONNECT_FAIL        = -1,	//连接失败
	E_WN_CLIENT_SEND_AUTH_INFO_FAIL = -2,	//发送验证信息失败
	E_WN_CLIENT_RECV_AUTH_INFO_FAIL = -3,	//接收验证确认信息失败
	E_WN_CLIENT_SEND_HB_FAIL        = -4,	//发送心跳失败
	E_WN_CLIENT_RECV_HEAD_FAIL      = -5,	//接收包头失败
	E_WN_CLIENT_RECV_BODY_FAIL      = -6,	//接收包体失败
	E_WN_CLIENT_SEND_DATA_FAIL      = -7,	//发送数据失败
	E_WN_CLIENT_SEND_LOGOUT_FAIL    = -8,	//发送注销失败
	E_WN_CLIENT_CONNECT_TIMEOUT     = -9,	//连接超时
	E_WN_CLIENT_RECV_TIMEOUT        = -10,	//接收超时
	E_WN_CLIENT_INVALID_IP          = -11,	//无效IP
	E_WN_CLIENT_INVALID_PORT        = -12,	//无效端口
	E_WN_CLIENT_RECV_NOMDF_FAIL     = -13,	//接收非MDF协议数据失败
	E_WN_CLIENT_RECV_NO_HB			= -14,	//长时间没有接收到心跳
	E_WN_CLIENT_CLOSE_SOCKET		= -15,	//CloseSocket
	E_WN_CLIENT_APP_EXIT            = -16,
	E_WN_CLIENT_UNKNOWN             = -17,
	E_WN_CLIENT_IOSERVICE_EXCEPTION = -18
};


class CConnection : public boost::enable_shared_from_this<CConnection>
{
public:
	CConnection(ITCPClient* pClient);
	virtual ~CConnection(void);

public:
	//连接MDF协议的TCP服务器
	void ConnectMDFServer( const char *szIP, unsigned short usPort, bool bAuth=false, int nTimeout = 5 );

	//发送数据给MDF协议的服务器（与ConnectMDFServer配对使用）
	bool SendDataToMDFServer( const unsigned char * pucData, unsigned int unDataSize );

	//连接非MDF协议的TCP服务器（兼容现有的服务器而设计）
	void ConnectOtherServer( const char *szIP, unsigned short usPort, bool bAuth=false, int nTimeout = 5 );

	//发送数据给非MDF协议的服务器（与ConnectOtherServer配对使用，兼容现有的服务器而设计）
	bool SendDataToOtherServer( const unsigned char * pucData, unsigned int unDataSize );

	//与服务器断开网络连接
	void CloseSocket(bool bExitFlag);

private:
	void OnSendMDFData(sock_pt sock,packet_pt pPacket);

	void OnSendOtherData(sock_pt sock,nonmdfpacket_pt pPacket);

	//异步消息处理循环
	void Run();

	//连接MDFServer超时回调函数
	void connect_timer_handler(sock_pt sock,const boost::system::error_code& ec);

	//异步连接回调函数
	void connect_handler(sock_pt sock,const boost::system::error_code& ec,const char *szIP, unsigned short usPort,bool bAuth);

	//发送数据的回调函数
	void send_data_handler(sock_pt sock,const boost::system::error_code& ec, size_t bytes_transferred, packet_pt pPacket, unsigned int unDataLen);

	void send_mdfdata_handler(sock_pt sock,const boost::system::error_code& ec, size_t bytes_transferred, packet_pt pPacket);

	void send_nonmdfdata_handler(sock_pt sock,const boost::system::error_code& ec, size_t bytes_transferred, nonmdfpacket_pt pPacket);

	//发送心跳包的回调函数
	void send_hb_handler(sock_pt sock,const boost::system::error_code& ec, size_t bytes_transferred, packet_pt pPacket, unsigned int unDataLen);

	//接收数据循环
	void RecvData( sock_pt sock );

	//接收包头的回调函数
	void recv_head_handler(sock_pt sock,const boost::system::error_code& ec,  size_t bytes_transferred/*, boost::shared_ptr<unsigned char> btHead*/);

	//接收包头超时回调函数
	//void read_timer_handler(sock_pt sock,const system::error_code& ec);

	//接收包体的回调函数
	void recv_body_handler(sock_pt sock,const boost::system::error_code& ec, size_t bytes_transferred, unsigned char btType, /*boost::shared_ptr<unsigned char> btBody,*/unsigned int ulLength);

	//错误处理  
	void disconnect_handler(sock_pt sock,int nErrorCode);

	//发送心跳定时器函数
	void HB_timer_handler(sock_pt sock,const boost::system::error_code& ec);

	//---------------------------   处理连接非MDF Server   -----------------------
	//连接非MDFServer回调函数
	void connect_NOT_MDF_handler(sock_pt sock,const boost::system::error_code& ec, const char *szIP, unsigned short usPort,bool bAuth);

	//接收非MDF Server数据循环
	void RecvNOTMDFData( sock_pt sock );

	//接收非MDF Server数据回调
	//void recv_NOT_MDF_handler( sock_pt sock, boost::shared_ptr<unsigned char> btBuffer, const boost::system::error_code& ec, std::size_t bytes_transferred );
	void recv_NOT_MDF_handler( sock_pt sock, const boost::system::error_code& ec, std::size_t bytes_transferred );

	//发送数据给非MDF Server的回调函数
	void send_NOT_MDF_data_handler(sock_pt sock, const boost::system::error_code& ec, std::size_t bytes_transferred, unsigned int ulDataSize);

	//心跳检测定时器的回调函数
	void check_hb_timer_handler(sock_pt sock, const boost::system::error_code& ec);

	//关闭socket
	void do_close( bool bExitFlag );

	//创建io_service线程池的线程函数
	void BeginRunThread();

public:
	//通知对象
	//ITCPClientNotify* m_pTCPClientNotifyObj;

	ITCPClient * m_pClient;

public:

	//连接标志
	bool m_bConnect;	

	//退出APP标志
	bool m_bAppExit;	

	//客户端io_service
	boost::shared_ptr<boost::asio::io_service> m_pIO;

	//客户端socket
	sock_pt m_socket;

	//心跳间隔
	int m_nHBInterval;

	//最后接收时间
	long m_tLastRecv;

	//最后发送时间
	long m_tLastSend;

	//心跳线程
	boost::thread m_HBThread;

	//数据接收线程
	boost::thread m_RecvThread;

	//接收超时定时器
	//shared_ptr<deadline_timer> m_ReadTimer;

	//连接超时定时器
	boost::shared_ptr<boost::asio::deadline_timer> m_ConnectTimer;

	//发送心跳定时器
	boost::shared_ptr<boost::asio::deadline_timer> m_HBTimer;

	//心跳检测定时器
	boost::shared_ptr<boost::asio::deadline_timer> m_HBCheckTimer;

	//io_service线程组大小
	int m_nThreadPoolSize;

	//io_service线程组
	boost::thread_group m_ThreadGroup;

	boost::asio::strand* m_strand;

	boost::crc_32_type m_crc32;

	int m_nMaxBodyLength;

	int m_nErrorCode;

	//unsigned char* pCRCBuffer;

	boost::mutex m_senddeq_mutex;
	std::deque<packet_pt> m_send_deq;
	boost::condition_variable m_cond_sendnotify;

	unsigned char* m_pchHead;
	unsigned char* m_pchBody;

private:
	std::list<packet_pt> m_listSendData;
	std::list<nonmdfpacket_pt> m_listSendData_NonMdf;

};

typedef boost::shared_ptr<CConnection> connection_pt;

#endif

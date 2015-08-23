#ifndef _MDFCLIENT_INCLUDE_20110526_Y_
#define _MDFCLIENT_INCLUDE_20110526_Y_

#pragma once
#include "Common.h"
#include "LogWriter.h"
#include <boost/enable_shared_from_this.hpp>
#include "ITigerNet.h"

//心跳发送间隔
#define HB_SEND_PERIOD            15
#define RECV_BUFFER_SIZE		  1024 * 1024

class CTCPClient: public boost::enable_shared_from_this<CTCPClient>
{
public:
	CTCPClient(boost::asio::io_service& io, boost::asio::ip::tcp::socket* sock, ITCPServer* pServer){};
	CTCPClient(boost::asio::io_service& io, boost::shared_ptr<boost::asio::ip::tcp::socket> sock, ITCPServer* pServer, boost::asio::strand* pStrand );
	~CTCPClient();

	//发送数据
	//void SendData(bool bMDF, unsigned char * pucData, unsigned int unDataSize);
	void OnSendMDFData(sock_pt pSock, packet_pt pPacket);
	void OnSendOtherData(sock_pt pSock, nonmdfpacket_pt pPacket);

	//启动心跳线程和数据接收
	void Start( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, bool bMDF = true );

	//发送心跳的回调函数
	void send_hb_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec);

	//接收数据
	void RecvData( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, bool bMDF=true );

	//接收非MDF协议数据回调函数
	void recv_non_mdf_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec, const unsigned char* btData, size_t bytes_transferred);

	//接收包头的回调函数
	void recv_head_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec, unsigned char* pucHead);

	//接收包体的回调函数
	void recv_body_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec,size_t  bytes_transferred, unsigned char ucType, unsigned long ulLength);

	//错误处理
	void disconnect_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, int nErrorCode );

	//发送心跳包的定时器函数
	void HB_timer_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec);

	//心跳检测定时器的回调函数
	void check_hb_timer_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec);

	//断开该Socket连接（外部调用触发）
	void CloseSocket();

	void send_mdfdata_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec, packet_pt pPacket);

	void send_nonmdfdata_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec, nonmdfpacket_pt pPacket);

	void SendPacket();

public:
	//用于数据传输的socket
	//ip::tcp::socket* m_socket;
	boost::shared_ptr<boost::asio::ip::tcp::socket> m_ptrSock;

	//服务器实例
	//CTigerServer* m_pServer;//ITCPServer
	ITCPServer * m_pServer;

	//服务端endpoint
	boost::asio::ip::tcp::endpoint m_ep;

	//心跳线程
	boost::thread m_HBThread;

	//接收线程
	boost::thread m_RecvThread;

	//心跳线程标志
	bool m_bHBThread;

	//接收线程标志
	bool m_bRecvThread;

	//最后接收心跳时间
	long m_tLastRecvHB;

	//最后接收时间
	long m_tLastRecv;

	//最后发送时间
	long m_tLastSend;

	//退出标志
	bool m_bExit;

	//接收数据超时的定时器
	//deadline_timer* m_ReadTimer;

	//发送心跳定时器
	boost::asio::deadline_timer* m_HBTimer;

	//心跳检测定时器
	boost::asio::deadline_timer* m_HBCheckTimer;

	//mutex m_socket_mutex;

	boost::mutex m_timer_mutex;

	boost::mutex m_exit_mutex;

	boost::asio::strand* m_strand; //modify

	unsigned char* m_pchHead;
	unsigned char* m_pchBody;

	unsigned char* m_pRecvBuffer;

	//unsigned char* pCRCBuffer;

	boost::crc_32_type m_crc32;

	int m_nMaxBodyLength;

private:
	std::list<packet_pt> m_listSendData;
	std::list<nonmdfpacket_pt> m_listSendData_NonMdf;

};

#endif

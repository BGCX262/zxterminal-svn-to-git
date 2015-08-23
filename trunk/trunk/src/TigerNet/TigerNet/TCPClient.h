#ifndef _MDFCLIENT_INCLUDE_20110526_Y_
#define _MDFCLIENT_INCLUDE_20110526_Y_

#pragma once
#include "Common.h"
#include "LogWriter.h"
#include <boost/enable_shared_from_this.hpp>
#include "ITigerNet.h"

//�������ͼ��
#define HB_SEND_PERIOD            15
#define RECV_BUFFER_SIZE		  1024 * 1024

class CTCPClient: public boost::enable_shared_from_this<CTCPClient>
{
public:
	CTCPClient(boost::asio::io_service& io, boost::asio::ip::tcp::socket* sock, ITCPServer* pServer){};
	CTCPClient(boost::asio::io_service& io, boost::shared_ptr<boost::asio::ip::tcp::socket> sock, ITCPServer* pServer, boost::asio::strand* pStrand );
	~CTCPClient();

	//��������
	//void SendData(bool bMDF, unsigned char * pucData, unsigned int unDataSize);
	void OnSendMDFData(sock_pt pSock, packet_pt pPacket);
	void OnSendOtherData(sock_pt pSock, nonmdfpacket_pt pPacket);

	//���������̺߳����ݽ���
	void Start( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, bool bMDF = true );

	//���������Ļص�����
	void send_hb_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec);

	//��������
	void RecvData( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, bool bMDF=true );

	//���շ�MDFЭ�����ݻص�����
	void recv_non_mdf_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec, const unsigned char* btData, size_t bytes_transferred);

	//���հ�ͷ�Ļص�����
	void recv_head_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec, unsigned char* pucHead);

	//���հ���Ļص�����
	void recv_body_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec,size_t  bytes_transferred, unsigned char ucType, unsigned long ulLength);

	//������
	void disconnect_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, int nErrorCode );

	//�����������Ķ�ʱ������
	void HB_timer_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec);

	//������ⶨʱ���Ļص�����
	void check_hb_timer_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec);

	//�Ͽ���Socket���ӣ��ⲿ���ô�����
	void CloseSocket();

	void send_mdfdata_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec, packet_pt pPacket);

	void send_nonmdfdata_handler( boost::shared_ptr<boost::asio::ip::tcp::socket> sock, const boost::system::error_code& ec, nonmdfpacket_pt pPacket);

	void SendPacket();

public:
	//�������ݴ����socket
	//ip::tcp::socket* m_socket;
	boost::shared_ptr<boost::asio::ip::tcp::socket> m_ptrSock;

	//������ʵ��
	//CTigerServer* m_pServer;//ITCPServer
	ITCPServer * m_pServer;

	//�����endpoint
	boost::asio::ip::tcp::endpoint m_ep;

	//�����߳�
	boost::thread m_HBThread;

	//�����߳�
	boost::thread m_RecvThread;

	//�����̱߳�־
	bool m_bHBThread;

	//�����̱߳�־
	bool m_bRecvThread;

	//����������ʱ��
	long m_tLastRecvHB;

	//������ʱ��
	long m_tLastRecv;

	//�����ʱ��
	long m_tLastSend;

	//�˳���־
	bool m_bExit;

	//�������ݳ�ʱ�Ķ�ʱ��
	//deadline_timer* m_ReadTimer;

	//����������ʱ��
	boost::asio::deadline_timer* m_HBTimer;

	//������ⶨʱ��
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

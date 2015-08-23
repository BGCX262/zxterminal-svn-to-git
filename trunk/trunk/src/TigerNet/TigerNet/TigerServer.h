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
	E_WN_SERVER_ACCEPT_OK				= 0,	//�������ӳɹ�
	E_WN_SERVER_ACCEPT_ADDR_USED        = -1,
	E_WN_SERVER_ACCEPT_ACCEPT_FAIL		= -2,	//��������ʧ��
	E_WN_SERVER_ACCEPT_SENDLOGIN_FAIL	= -3,	//���͵�¼��֤ȷ����Ϣʧ��
	E_WN_SERVER_ACCEPT_RECVLOGIN_FAIL	= -4,	//���յ�½��֤��Ϣʧ��
	
	E_WN_SERVER_SEND_DATA_FAIL			= -5,	//��������ʧ��
	E_WN_SERVER_SEND_HB_FAIL			= -6,	//��������ʧ��
	E_WN_SERVER_RECV_HEAD_FAIL			= -7,	//���հ�ͷʧ��
	E_WN_SERVER_RECV_BODY_FAIL			= -8,	//���հ���ʧ��
	E_WN_SERVER_RECV_TIMEOUT			= -9,	//���ճ�ʱ
	E_WN_SERVER_RECV_NOMDF_FAIL			= -10,	//���շ�MDF����ʧ��
	E_WN_SERVER_RECV_NO_HB				= -11,	//��ʱ��û�н��յ�����
	E_WN_SERVER_NOTIFY_CLOSE			= -12,	//�ⲿ֪ͨ�Ͽ�����
	E_WN_SERVER_TOO_MUCH_DATA			= -13	//���ݶѻ�
};

class CTigerServer : public ITCPServer
{
public:
	CTigerServer(ITCPServerNotify* pTCPServerNotifyObj);
	~CTigerServer(void);

public:
	//��ָ���˿�����TCP����������
	virtual bool MDFServerListen( unsigned short usPort,bool bAuth );

	//�������ݸ�ָ���Ŀͻ���
	virtual bool SendDataToMDFClient( unsigned __int64 unClientID, const unsigned char * pucData, unsigned int unDataSize );

	//��ָ���˿�������MDFЭ���TCP����������������������
	virtual bool OtherServerListen( unsigned short usPort,bool bAuth=false );

	//�������ݸ���MDF�ͻ��ˣ���OtherServerListen���ʹ�ã�
	virtual bool SendDataToOtherClient( unsigned __int64 unClientID, const unsigned char * pucData, unsigned int unDataSize );

	//�ر�TCP Server
	virtual void CloseSocket();

public:
	//���ܿͻ������ӵĻص�����
	void accept_handler(const boost::system::error_code& ec, boost::shared_ptr<boost::asio::ip::tcp::socket> sock, bool bMDF=true);

	//�첽��Ϣ����ѭ���̺߳���
	void Run();

	//����io_service�߳�����̺߳���
	void BeginRunThread();

	void do_close();

	void CloseAllClients();

	void DeleteAllClients();

public:
	//������
	void server_disconnect_handler(unsigned __int64 unClientID, boost::shared_ptr<boost::asio::ip::tcp::socket> sock, int nErrorCode);

public:
	//֪ͨ����
	ITCPServerNotify* m_pTCPServerNotifyObj;

public:
	//����˵�io_service
	//shared_ptr<io_service> m_pIO;
	boost::asio::io_service* m_pIO;

	//�����acceptor
	boost::asio::ip::tcp::acceptor* m_acceptor;

	//�ͻ�������
	std::map<unsigned __int64, boost::shared_ptr<CTCPClient> > m_mapClient;

	//�ͻ��������Ļ�����
	boost::mutex m_mapClient_mutex;

	//��֤��־
	bool m_bAuth;

	//�������
	int m_nHBInterval;

	//������ʱ��
	long m_tLastRecv;

	//�����ʱ��
	long m_tLastSend;

	//�˳���־
	bool m_bExit;
	
	//�˳�APP��־
	bool m_bAppExit;	

    //io_service�̳߳��߳���Ŀ
	int m_nThreadPoolSize;

	//io_service�߳���
	boost::thread_group m_ThreadGroup;

	boost::asio::strand* m_strand;

	boost::shared_ptr<boost::asio::io_service::work> m_work;

	/// The pool of io_service objects used to perform asynchronous operations.
	io_service_pool io_service_pool_;

};

#endif





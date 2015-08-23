#ifndef _CONN_INCLUDE_20110617_
#define _CONN_INCLUDE_20110617_

#include "Common.h"
#include "ITigerNet.h"

#ifdef __LOG__
#include "LogWriter.h" 
#endif

#define HEADER_LENGTH        8
#define MAX_BODY_LENGTH      1024 * 1024

//�������ͼ��
#define HB_SEND_PERIOD            15

enum E_WN_CLIENT_ERROR_CODE
{
	E_WN_CLIENT_CONNECT_OK          = 0,	//���ӳ�ʱ
	E_WN_CLIENT_CONNECT_FAIL        = -1,	//����ʧ��
	E_WN_CLIENT_SEND_AUTH_INFO_FAIL = -2,	//������֤��Ϣʧ��
	E_WN_CLIENT_RECV_AUTH_INFO_FAIL = -3,	//������֤ȷ����Ϣʧ��
	E_WN_CLIENT_SEND_HB_FAIL        = -4,	//��������ʧ��
	E_WN_CLIENT_RECV_HEAD_FAIL      = -5,	//���հ�ͷʧ��
	E_WN_CLIENT_RECV_BODY_FAIL      = -6,	//���հ���ʧ��
	E_WN_CLIENT_SEND_DATA_FAIL      = -7,	//��������ʧ��
	E_WN_CLIENT_SEND_LOGOUT_FAIL    = -8,	//����ע��ʧ��
	E_WN_CLIENT_CONNECT_TIMEOUT     = -9,	//���ӳ�ʱ
	E_WN_CLIENT_RECV_TIMEOUT        = -10,	//���ճ�ʱ
	E_WN_CLIENT_INVALID_IP          = -11,	//��ЧIP
	E_WN_CLIENT_INVALID_PORT        = -12,	//��Ч�˿�
	E_WN_CLIENT_RECV_NOMDF_FAIL     = -13,	//���շ�MDFЭ������ʧ��
	E_WN_CLIENT_RECV_NO_HB			= -14,	//��ʱ��û�н��յ�����
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
	//����MDFЭ���TCP������
	void ConnectMDFServer( const char *szIP, unsigned short usPort, bool bAuth=false, int nTimeout = 5 );

	//�������ݸ�MDFЭ��ķ���������ConnectMDFServer���ʹ�ã�
	bool SendDataToMDFServer( const unsigned char * pucData, unsigned int unDataSize );

	//���ӷ�MDFЭ���TCP���������������еķ���������ƣ�
	void ConnectOtherServer( const char *szIP, unsigned short usPort, bool bAuth=false, int nTimeout = 5 );

	//�������ݸ���MDFЭ��ķ���������ConnectOtherServer���ʹ�ã��������еķ���������ƣ�
	bool SendDataToOtherServer( const unsigned char * pucData, unsigned int unDataSize );

	//��������Ͽ���������
	void CloseSocket(bool bExitFlag);

private:
	void OnSendMDFData(sock_pt sock,packet_pt pPacket);

	void OnSendOtherData(sock_pt sock,nonmdfpacket_pt pPacket);

	//�첽��Ϣ����ѭ��
	void Run();

	//����MDFServer��ʱ�ص�����
	void connect_timer_handler(sock_pt sock,const boost::system::error_code& ec);

	//�첽���ӻص�����
	void connect_handler(sock_pt sock,const boost::system::error_code& ec,const char *szIP, unsigned short usPort,bool bAuth);

	//�������ݵĻص�����
	void send_data_handler(sock_pt sock,const boost::system::error_code& ec, size_t bytes_transferred, packet_pt pPacket, unsigned int unDataLen);

	void send_mdfdata_handler(sock_pt sock,const boost::system::error_code& ec, size_t bytes_transferred, packet_pt pPacket);

	void send_nonmdfdata_handler(sock_pt sock,const boost::system::error_code& ec, size_t bytes_transferred, nonmdfpacket_pt pPacket);

	//�����������Ļص�����
	void send_hb_handler(sock_pt sock,const boost::system::error_code& ec, size_t bytes_transferred, packet_pt pPacket, unsigned int unDataLen);

	//��������ѭ��
	void RecvData( sock_pt sock );

	//���հ�ͷ�Ļص�����
	void recv_head_handler(sock_pt sock,const boost::system::error_code& ec,  size_t bytes_transferred/*, boost::shared_ptr<unsigned char> btHead*/);

	//���հ�ͷ��ʱ�ص�����
	//void read_timer_handler(sock_pt sock,const system::error_code& ec);

	//���հ���Ļص�����
	void recv_body_handler(sock_pt sock,const boost::system::error_code& ec, size_t bytes_transferred, unsigned char btType, /*boost::shared_ptr<unsigned char> btBody,*/unsigned int ulLength);

	//������  
	void disconnect_handler(sock_pt sock,int nErrorCode);

	//����������ʱ������
	void HB_timer_handler(sock_pt sock,const boost::system::error_code& ec);

	//---------------------------   �������ӷ�MDF Server   -----------------------
	//���ӷ�MDFServer�ص�����
	void connect_NOT_MDF_handler(sock_pt sock,const boost::system::error_code& ec, const char *szIP, unsigned short usPort,bool bAuth);

	//���շ�MDF Server����ѭ��
	void RecvNOTMDFData( sock_pt sock );

	//���շ�MDF Server���ݻص�
	//void recv_NOT_MDF_handler( sock_pt sock, boost::shared_ptr<unsigned char> btBuffer, const boost::system::error_code& ec, std::size_t bytes_transferred );
	void recv_NOT_MDF_handler( sock_pt sock, const boost::system::error_code& ec, std::size_t bytes_transferred );

	//�������ݸ���MDF Server�Ļص�����
	void send_NOT_MDF_data_handler(sock_pt sock, const boost::system::error_code& ec, std::size_t bytes_transferred, unsigned int ulDataSize);

	//������ⶨʱ���Ļص�����
	void check_hb_timer_handler(sock_pt sock, const boost::system::error_code& ec);

	//�ر�socket
	void do_close( bool bExitFlag );

	//����io_service�̳߳ص��̺߳���
	void BeginRunThread();

public:
	//֪ͨ����
	//ITCPClientNotify* m_pTCPClientNotifyObj;

	ITCPClient * m_pClient;

public:

	//���ӱ�־
	bool m_bConnect;	

	//�˳�APP��־
	bool m_bAppExit;	

	//�ͻ���io_service
	boost::shared_ptr<boost::asio::io_service> m_pIO;

	//�ͻ���socket
	sock_pt m_socket;

	//�������
	int m_nHBInterval;

	//������ʱ��
	long m_tLastRecv;

	//�����ʱ��
	long m_tLastSend;

	//�����߳�
	boost::thread m_HBThread;

	//���ݽ����߳�
	boost::thread m_RecvThread;

	//���ճ�ʱ��ʱ��
	//shared_ptr<deadline_timer> m_ReadTimer;

	//���ӳ�ʱ��ʱ��
	boost::shared_ptr<boost::asio::deadline_timer> m_ConnectTimer;

	//����������ʱ��
	boost::shared_ptr<boost::asio::deadline_timer> m_HBTimer;

	//������ⶨʱ��
	boost::shared_ptr<boost::asio::deadline_timer> m_HBCheckTimer;

	//io_service�߳����С
	int m_nThreadPoolSize;

	//io_service�߳���
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

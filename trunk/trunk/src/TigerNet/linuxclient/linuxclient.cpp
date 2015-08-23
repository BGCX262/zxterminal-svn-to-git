
#include "../../../../Include/MDFNetModule/ITCPClient.h"

#ifdef _DEBUG
#pragma comment(lib,"../../../../lib/debug/CommonClientd.lib")
#else
#pragma comment(lib,"../../../../lib/release/CommonClient.lib")
#endif

#define __int64  long long

#include "Common.h"

class CClient : public ITCPClientNotify
{

public:
	//连接服务器回调函数，由nStatus指定连接是否成功。
	virtual void OnConnectServer( int nStatus );

	//从服务器接收到数据回调
	virtual void OnRecvServerData( unsigned char * pucData, unsigned int unDataSize );

	//从服务器接收数据异常回调
	virtual void OnServerDataError();

	//Socket连接断开回调
	virtual void OnServerDisconnect( int nStatus );

	void Initialize();

	void Run();

	void Routine();

	ITCPClient *m_pTCPClientObj;
	
	bool m_bMDF;
	__int64 m_ulSended;
	__int64 m_ulReceived;


	bool m_bExit;	//退出APP标志
	bool m_bNotifyExit;

	//用于开始转发数据的条件变量
	boost::condition_variable m_condRelay;

	//用于通知转发的互斥量
	boost::mutex m_notify_mutex;
};

void CClient::Initialize()
{
	m_pTCPClientObj = NULL;
	m_bMDF = true;
	m_ulSended = 0;
	m_ulReceived = 0;

	if ( m_pTCPClientObj==NULL )
	{
		m_pTCPClientObj = CreateMDFTCPClientInstance( this );
	}

	boost::thread tClient(&CClient::Run, this);
	tClient.join();
}

void CClient::Run()
{
	if (m_bMDF)
	{
		m_pTCPClientObj->ConnectMDFServer("10.100.2.52", 8888 );

	}
	else
	{
		m_pTCPClientObj->ConnectOtherServer( "192.168.142.131", 5555 );
	}

	{
		//boost::mutex::scoped_lock notify_lock(m_notify_mutex);
		//m_condRelay.wait(notify_lock);
	}

}

void CClient::Routine()
{
	unsigned char ucMsg[10];
	memcpy(ucMsg, "1234567890", 10);

	while (1)
	{
		boost::this_thread::sleep(boost::posix_time::seconds(1));

		if (m_pTCPClientObj)
		{
			m_pTCPClientObj->SendDataToMDFServer( ucMsg, 10 );

			m_ulSended += 10;
			printf("Send[%lu Byte]\n", m_ulSended);
		}
	}
}

//连接服务器回调函数，由nStatus指定连接是否成功。
void CClient::OnConnectServer( int nStatus )
{
	printf("Connect status=%d\n", nStatus );

	switch ( nStatus )
	{
	case -1://网络断开处理

		break;

	case 0:
		//GetDlgItem( IDC_BTN_SEND )->EnableWindow( TRUE );
		//boost::thread tSend(&CClient::Routine, this);
		break;

	case -15:
		//GetDlgItem( IDC_BTN_CONNECT )->EnableWindow( TRUE );
		break;

	default:
		break;
	}

	//if (nStatus == 0)
	//{
	//	boost::thread tSend(&CClient::Routine, this);
	//}

}

//从服务器接收到数据回调
void CClient::OnRecvServerData( unsigned char * pucData, unsigned int unDataSize )
{
	m_ulReceived += unDataSize;

	time_t now;
	time(&now);
	struct tm* p = localtime(&now);

	printf("[%02d:%02d:%02d] Recv: Total[%lu Byte] Current[%lu Byte]\n", p->tm_hour, p->tm_min, p->tm_sec, m_ulReceived, unDataSize);
}

//Socket连接断开回调
void CClient::OnServerDisconnect( int nStatus )
{
	time_t now;
	time(&now);
	struct tm* p = localtime(&now);
	printf("%02d:%02d:%02d OnServerDisconnect\n", p->tm_hour, p->tm_min, p->tm_sec );

	if ( nStatus == -16 )	//SDK通知资源释放完毕
	{
		if ( m_bExit )	//发出退出APP请求
		{
			if ( m_pTCPClientObj )
			{
				delete m_pTCPClientObj;
				m_pTCPClientObj = NULL;
			}
			//__super::OnCancel();

			return;		
		}

	}
}

//从服务器接收数据异常回调
void CClient::OnServerDataError()
{
	time_t now;
	time(&now);
	struct tm* p = localtime(&now);
	printf("%02d:%02d:%02d OnServerDataError\n", p->tm_hour, p->tm_min, p->tm_sec );
}

int main(int argc, char** argv[])
{
	CClient client;
	client.Initialize();

	

	getchar();

	return 0;
}

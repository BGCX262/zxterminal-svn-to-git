
#ifdef _DEBUG
#pragma comment(lib,"../../../../lib/debug/CommonServerd.lib")
#else
#pragma comment(lib,"../../../../lib/release/CommonServer.lib")
#endif

#include "Common.h"
#include "../../../../Include/MDFNetModule/ITCPServer.h"

using namespace std;
typedef map<unsigned __int64,string> mapClient;

#define __int64  long long

// CNetTestDlg dialog
class CServer : public ITCPServerNotify
{
public:
	//接受客户端连接回调
	virtual void OnAccept( unsigned __int64 unClientID, const char * ClientIP , unsigned short usPort );

	//从客户端接收到数据回调
	virtual void OnRecvClientData( unsigned __int64 unClientID, const unsigned char * pucData, unsigned int unDataSize );

	//从客户端接收数据异常回调
	virtual void OnClientDataError(unsigned __int64 unClientID);

	//断开客户端网络连接回调
	virtual void OnClientDisconnect( unsigned __int64 unClientID );

	void Initialize();

	void Run();

	ITCPServer *m_pTCPServerObj;

	mapClient m_mapClients;
	boost::mutex m_mapMutex;
	bool m_bMDF;
	__int64 m_ulSended;
	__int64 m_ulReceived;

	//用于开始转发数据的条件变量
	boost::condition_variable m_condRelay;

	//用于通知转发的互斥量
	boost::mutex m_notify_mutex;
};

void CServer::Initialize()
{
	m_pTCPServerObj = NULL;
	m_bMDF = true;
	m_ulSended = 0;
	m_ulReceived = 0;

	if ( m_pTCPServerObj==NULL )
	{
		m_pTCPServerObj = CreateMDFTCPServerInstance( this );
	}

	boost::thread tServer(&CServer::Run, this);
	tServer.join();
}

void CServer::Run()
{
	if (m_bMDF)
	{
		if (!m_pTCPServerObj->MDFServerListen( 5555 ,false))
		{
			printf("端口已被占用\n");
		}
	}
	else
	{
		if (!m_pTCPServerObj->OtherServerListen( 5555 ,false))
		{
			printf("端口已被占用\n");
		}
	}

	{
		boost::mutex::scoped_lock notify_lock(m_notify_mutex);
		m_condRelay.wait(notify_lock);
	}

}

//接受客户端连接回调
void CServer::OnAccept( unsigned __int64 unClientID, const char * ClientIP, unsigned short usPort  )
{
	boost::mutex::scoped_lock lock(m_mapMutex);

	m_mapClients.insert( mapClient::value_type(unClientID, string(ClientIP)) );
	printf("[%s]connected Num[%d]\n", ClientIP, m_mapClients.size());
}

//从服务器接收到数据回调
void CServer::OnRecvClientData( unsigned __int64 unClientID, const unsigned char * pucData, unsigned int unDataSize )
{
	//char *szData = new char[unDataSize+1];
	//memcpy( szData, pucData, unDataSize );
	//szData[unDataSize] = '\0';

	m_ulReceived += unDataSize;
	printf("Recv[%lu Byte]\n", m_ulReceived);

	//if (m_bMDF)
	//{
	//	m_pTCPServerObj->SendDataToMDFClient( unClientID,/* (unsigned char*)*/pucData, unDataSize );
	//} 
	//else
	//{
	//	m_pTCPServerObj->SendDataToOtherClient( unClientID, /*(unsigned char*)*/pucData, unDataSize );
	//}

	//m_ulSended += unDataSize;
	//printf("Send[%lu Byte]\n", m_ulSended);


	//delete[] szData;
	//szData = NULL;
}

//Socket连接断开回调
void CServer::OnClientDisconnect(  unsigned __int64 unClientID )
{
	boost::mutex::scoped_lock lock(m_mapMutex);


	string strIP;
	mapClient::iterator it = m_mapClients.find( unClientID );
	if ( it!=m_mapClients.end() )
	{
		strIP = (*it).second;
		m_mapClients.erase( it );
		printf("[%s]disconnect Num[%d]\n", strIP.c_str(), m_mapClients.size());
	}

	//if (unClientID == 0)
	//{
	//	delete m_pTCPServerObj;
	//	m_pTCPServerObj = NULL;
	//	__super::OnCancel();
	//}
}

//从服务器接收数据异常回调
void CServer::OnClientDataError(unsigned __int64 unClientID)
{

}

int main(int argc, char** argv[])
{
	CServer server;
	server.Initialize();

	getchar();

	return 0;
}


// CommonClient.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "TigerClient.h"

#ifdef __LOG__
//日志
extern CLogWriter m_LogWriter;
#endif

//==================================================================
//函数名：CreateMDFTCPClientInstance  
//作者： yin hui   
//日期： 20110128  
//功能：  创建CCommonClient实例的接口函数  
//输入参数：  ITCPClientNotify* pTCPClientNotifyObj
//返回值：  CCommonClient实例
//修改记录：取消输入参数boost::asio::io_service& io
//==================================================================

ITCPClient* CreateMDFTCPClientInstance(  ITCPClientNotify* pTCPClientNotifyObj )
{
	if ( pTCPClientNotifyObj==NULL )
		return NULL;

#ifdef __LOG__
	if (m_LogWriter.Initialize())
	{
		CLogWriter::Log(WN_LOG_LEVEL_INFO,"------------------------");
		CLogWriter::Log(WN_LOG_LEVEL_INFO,"[Initialize log succeed]");
	} 
#endif

	return new CTigerClient( pTCPClientNotifyObj );
}

//==================================================================
//函数名： CCommonClient 
//作者： yin hui   
//日期： 20110128  
//功能： CCommonClient构造函数，初始化类   
//输入参数：ITCPClientNotify* pTCPClientNotifyObj
//返回值：  无
//修改记录：取消输入参数boost::asio::io_service& io
//==================================================================
CTigerClient::CTigerClient(ITCPClientNotify* pTCPClientNotifyObj)
:m_pTCPClientNotifyObj(pTCPClientNotifyObj)
{

}

 //==================================================================
 //函数名：~CCommonClient  
 //作者： yin hui   
 //日期： 20110128   
 //功能： CCommonClient析构函数，释放资源，关闭socket   
 //输入参数：
 //返回值：  
 //修改记录：
 //==================================================================
CTigerClient::~CTigerClient(void)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::~CCommonClient] [IN]");
#endif

	if (m_pConn.get())
	{
		m_pConn.reset();
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::~CCommonClient] [OUT]");

	m_LogWriter.Uninitialize();
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
void CTigerClient::ConnectMDFServer( const char *szIP, unsigned short usPort,bool bAuth, unsigned int nTimeout)
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::ConnectMDFServer] [IN]");
#endif

	m_pConn = boost::shared_ptr<CConnection>(new CConnection(this));
	if (m_pConn.get())
	{
		m_pConn->ConnectMDFServer(szIP, usPort, bAuth, nTimeout);
	}
	

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::ConnectMDFServer] [OUT]\n");
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
void CTigerClient::ConnectOtherServer( const char *szIP, unsigned short usPort, bool bAuth, unsigned int nTimeout )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::ConnectOtherServer] [IN]");
#endif

	m_pConn = boost::shared_ptr<CConnection>(new CConnection(this));
	if (m_pConn.get())
	{
		m_pConn->ConnectOtherServer(szIP, usPort, bAuth, nTimeout);
	}
	
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::ConnectOtherServer] [OUT]\n");
#endif
}

//发送数据给非MDF协议的服务器
bool CTigerClient::SendDataToOtherServer( const unsigned char * pucData, unsigned int unDataSize )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::SendDataToOtherServer] [IN]");
#endif

	if (m_pConn.get())
	{
		m_pConn->SendDataToOtherServer(pucData, unDataSize);
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::SendDataToOtherServer] [OUT]\n");
#endif

	return true;
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
bool CTigerClient::SendDataToMDFServer( const unsigned char * pucData, unsigned int unDataSize )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::SendDataToMDFServer] [IN]");
#endif

	if (m_pConn.get())
	{
		m_pConn->SendDataToMDFServer(pucData, unDataSize);
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::SendDataToMDFServer] [OUT]\n");
#endif

	return true;
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
void CTigerClient::CloseSocket( bool bExitFlag )
{
#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::CloseSocket] [IN]");
#endif

	if (m_pConn.get())
	{
		m_pConn->CloseSocket(bExitFlag);
	}

#ifdef __LOG__
	CLogWriter::Log(WN_LOG_LEVEL_DEBUG,"[CCommonClient::CloseSocket] [OUT]\n");
#endif
}


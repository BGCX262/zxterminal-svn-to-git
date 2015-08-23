#ifndef _ITCP_INCLUDE_20110124_
#define _ITCP_INCLUDE_20110124_

#include "Common.h"
#include "ITigerNet.h"

#ifdef __LOG__
#include "LogWriter.h" 
#endif

#include "Connection.h"

class CTigerClient : public ITCPClient
{
public:
	CTigerClient(ITCPClientNotify* pTCPClientNotifyObj);
	virtual ~CTigerClient(void);

public:
	//连接MDF协议的TCP服务器
	virtual void ConnectMDFServer( const char *szIP, unsigned short usPort, bool bAuth=false, unsigned int nTimeout = 5 );

	//发送数据给MDF协议的服务器（与ConnectMDFServer配对使用）
	virtual bool SendDataToMDFServer( const unsigned char * pucData, unsigned int unDataSize );

	//连接非MDF协议的TCP服务器（兼容现有的服务器而设计）
	virtual void ConnectOtherServer( const char *szIP, unsigned short usPort, bool bAuth=false, unsigned int nTimeout = 5 );

	//发送数据给非MDF协议的服务器（与ConnectOtherServer配对使用，兼容现有的服务器而设计）
	virtual bool SendDataToOtherServer( const unsigned char * pucData, unsigned int unDataSize );

	//与服务器断开网络连接
	virtual void CloseSocket( bool bExitFlag=false );

public:
	//通知对象
	ITCPClientNotify* m_pTCPClientNotifyObj;

	connection_pt m_pConn;

	boost::mutex m_cond_exit_mutex;

	boost::condition_variable m_CondExitNotify;
};

#endif

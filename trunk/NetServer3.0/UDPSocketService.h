// SocketService.h: interface for the CSocketService class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UDPSOCKETSERVICE_H__5A758F39_5200_416B_A8F2_72D786B5D800__INCLUDED_)
#define AFX_UDPSOCKETSERVICE_H__5A758F39_5200_416B_A8F2_72D786B5D800__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "SystemInfo.h"
//#include "NetReadThread.h"
//#include "NetWorkThread.h"

#include "HighPerformanceUDPServer.h"

class CUDPSocketService  
{
public:
	int EndService();
	BOOL StartService();
	CUDPSocketService();
	virtual ~CUDPSocketService();
	int  fd;
	//NetReadThread   m_ReadThread;
	HighPerformanceUDPServer m_HPUDPServer;

private:
////	NetReadThread   m_ReadThread;
//	CNetWorkThread  m_WorkThread;
	
};

#endif // !defined(AFX_UDPSOCKETSERVICE_H__5A758F39_5200_416B_A8F2_72D786B5D800__INCLUDED_)

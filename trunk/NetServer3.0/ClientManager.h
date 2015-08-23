// ClientManager.h: interface for the CClientManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENTMANAGER_H__C6242132_F259_4F0F_9950_5AD79352B889__INCLUDED_)
#define AFX_CLIENTMANAGER_H__C6242132_F259_4F0F_9950_5AD79352B889__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "NetField.h"
//#include "Winsock.h"
#include <afxmt.h>

/*用来管理登陆的用户的*/
  

class CClientManager  
{
public:
	ClintNode  * GetClientNodeFromId(int index);
	ClintNode  * GetClintNode(int fd);
	int  GetUserCount();
	void GetClientofFd(const CString  &szUser,int &fd);
	void GetInfo(int fd,CString &szUserId, CString  &szIP, CString  &szTime);
	int FindNode(int  fd);
	BOOL DeletNode(int fd);
	BOOL AddNode(int fd, short iPort,CString szIP,CString  szUserId);
	CClientManager();
	virtual ~CClientManager();
private:
//	CMutex   m_Lock;
	CObArray m_aClientArray;


protected:
	
};

#endif // !defined(AFX_CLIENTMANAGER_H__C6242132_F259_4F0F_9950_5AD79352B889__INCLUDED_)

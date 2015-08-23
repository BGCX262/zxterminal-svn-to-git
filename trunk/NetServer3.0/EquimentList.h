// EquimentList.h: interface for the CEquimentList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EQUIMENTLIST_H__A44BFCFD_3B93_4E4E_9D3D_AAFEEE5B37FB__INCLUDED_)
#define AFX_EQUIMENTLIST_H__A44BFCFD_3B93_4E4E_9D3D_AAFEEE5B37FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Winsock2.h"
#include <afxmt.h>

/*记录设备信息的列表*/
typedef   struct  tagEquiNetInfo
{
	CString  szKey;  //记录设备的标识
	int       fd;
	struct  sockaddr_in   addr;
}EquiNetInfo,*pEquiNetInfo;




class CEquimentList : public CPtrList  
{
public:
	void UpdatUser(int fd, CString szKey,  struct sockaddr_in  addr);
	void AddUser(int  fd,CString  szKey, struct sockaddr_in  addr);
	EquiNetInfo  * Find(CString szKey);
	CEquimentList();
	virtual ~CEquimentList();
private:
	CMutex   m_lock;     
};

#endif // !defined(AFX_EQUIMENTLIST_H__A44BFCFD_3B93_4E4E_9D3D_AAFEEE5B37FB__INCLUDED_)

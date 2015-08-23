// ThreadMsgManager.h: interface for the CThreadMsgManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THREADMSGMANAGER_H__5B10F749_6990_4496_ABDA_0AB33259A5B6__INCLUDED_)
#define AFX_THREADMSGMANAGER_H__5B10F749_6990_4496_ABDA_0AB33259A5B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*对线程之间的消息包的管理*/
#include "NetField.h"
#include <afxmt.h>

class CThreadMsgManager  
{
public:
	void Clear();
	BOOL GetTheadMsg(ThreadMsg   &thMsg);
	void Add(ThreadMsg thMsg);  
	CThreadMsgManager();
	virtual ~CThreadMsgManager();
private:

	TEMP_THREADMSG  m_asThreadMsg;   // b保存消息的列表;
	CMutex   m_Lock;

};


#endif // !defined(AFX_THREADMSGMANAGER_H__5B10F749_6990_4496_ABDA_0AB33259A5B6__INCLUDED_)

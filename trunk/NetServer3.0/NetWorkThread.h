// NetWorkThread.h: interface for the CNetWorkThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETWORKTHREAD_H__F461E6E1_307A_4648_8C6F_35B8F9E342BE__INCLUDED_)
#define AFX_NETWORKTHREAD_H__F461E6E1_307A_4648_8C6F_35B8F9E342BE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*******************************************************/
/*负责网络数据的处理                                   */
/*******************************************************/

#include "Thread.h"
#include "NetField.h"

class CNetWorkThread : public Thread  
{
public:
	void ReleaseBuf(CNetNode *pNetData);
	BOOL GetNetMsg(ThreadMsg   &Msg);  //获取网络数据包;
	virtual  void Run();
	CNetWorkThread();
	virtual ~CNetWorkThread();
private:
	BOOL m_bRun;
};

#endif // !defined(AFX_NETWORKTHREAD_H__F461E6E1_307A_4648_8C6F_35B8F9E342BE__INCLUDED_)

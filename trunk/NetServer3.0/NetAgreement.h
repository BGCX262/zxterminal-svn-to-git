// NetAgreement.h: interface for the CNetAgreement class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETAGREEMENT_H__3A03E4CD_DA36_4404_AB08_0A91B1AD106D__INCLUDED_)
#define AFX_NETAGREEMENT_H__3A03E4CD_DA36_4404_AB08_0A91B1AD106D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "NetField.h"
#include "BaseNetAgreement.h"
/*�������Ҫ�Ǵ�������Э�飬��������������;*/

class CNetAgreement :public  CBaseNetAgreement 
{
public:
	void DealNetMsg();
	CNetAgreement(int fd, NetMsgHead* p, int iMsgLen);
	CNetAgreement();
	//char* m_pBufMsg;
	virtual ~CNetAgreement();
	//int m_fd;
	//NetMsgHead* m_pThreadMsg;
	//int m_MsgLen;
};

#endif // !defined(AFX_NETAGREEMENT_H__3A03E4CD_DA36_4404_AB08_0A91B1AD106D__INCLUDED_)

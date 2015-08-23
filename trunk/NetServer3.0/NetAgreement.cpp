// NetAgreement.cpp: implementation of the CNetAgreement class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetServer.h"
#include "NetAgreement.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetAgreement::CNetAgreement()
{
	m_pBufMsg=NULL;
}
CNetAgreement::CNetAgreement(int fd, NetMsgHead* p, int iMsgLen)
{
	m_MsgLen = iMsgLen;
	m_pThreadMsg = p;
	m_fd = fd;
	m_pBufMsg = NULL;
}

CNetAgreement::~CNetAgreement()
{
}

void CNetAgreement::DealNetMsg()
{
	if(!m_pThreadMsg)
	{
		return;
	}

	if(m_pThreadMsg->Version != VERSION)
	{
		//�汾���ԣ�������ʱ��������;
	}

	switch(m_pThreadMsg->MsgType)
	{
	case  MSGTYPE_COMMON:
		{
			this->DoCommMsg();
		}
		break; 

	case  MSGTYPE_GPS:   //�����Ƕ�λ��ص�����;
		{
			this->DoGpsMsg();
		}
		break;

	case MSGTYPE_GUARD:
		{
			//�Ͱ�ȫ�����豸���;
			this->DoGuardMsg();
		}
		break;

	default:
		break;
	}   
}

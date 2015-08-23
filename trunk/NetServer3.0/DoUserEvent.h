// DoUserEvent.h: interface for the CDoUserEvent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOUSEREVENT_H__4C7D9FF6_D766_4598_AEB5_7E959169B8A6__INCLUDED_)
#define AFX_DOUSEREVENT_H__4C7D9FF6_D766_4598_AEB5_7E959169B8A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*���������û����¼����࣬��¼�û��Ĳ���*/
class CDoUserEvent  
{
public:
	BOOL RecordUserEvent();
	//�õ��û����豸��������;
	CDoUserEvent(int iSocketIndex,CString  szEquimentID,int iCommandId,CString  szParma,int BSuccess);
		
	BOOL GetEuqimentCommandText(CString  szEuqimentId,int iCoomandId,CString  &szCommandText);
	//�õ��û���ID;
	BOOL GetUserId(int iSocketIndex,CString  &szuserId);
	CDoUserEvent();
	virtual ~CDoUserEvent();
	int m_iSocketIndex;
	CString  m_szEquimentID;
	int m_iCommandId;
	CString  m_szParma;
	int m_BeOK;
	CString m_UserIP;

};

#endif // !defined(AFX_DOUSEREVENT_H__4C7D9FF6_D766_4598_AEB5_7E959169B8A6__INCLUDED_)

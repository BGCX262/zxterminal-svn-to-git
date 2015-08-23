// GmemoryDataManager.h: interface for the CGmemoryDataManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GMEMORYDATAMANAGER_H__0EA5A2A6_B169_4E78_8663_C241F0A6400A__INCLUDED_)
#define AFX_GMEMORYDATAMANAGER_H__0EA5A2A6_B169_4E78_8663_C241F0A6400A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*�������е��ڴ�����*/
/*20060910*/
#include "CommField.h"
class CGmemoryDataManager  
{
public:
	const  CGpsAlarmDealWithInfo * FindGpsAlarmInfo(const CString &EuqimentKey, int iState,BOOL  bDB=TRUE);
	BOOL LoadGpsAlarmInfo();
	CEquimentUser  * GetEquimentUser(const  CString  &szEquimentId);
	int LoadEquimentUser();
	
	CEquimentInfo * FindEquimentInfo(CString szEquimnetId,BOOL  bDB=TRUE);
	int LoadAllEquimentInfo();
	

	CGmemoryDataManager();
	virtual ~CGmemoryDataManager();
private:
	CObArray       m_aEuqimentInfo;//�豸����Ϣ;
	CObArray       m_aEquimentUser;//�õ��豸���û�;
	CObArray       m_aGpsAlarmInfo; //GPS�豸�ı�����ϸ��Ϣ;

};

#endif // !defined(AFX_GMEMORYDATAMANAGER_H__0EA5A2A6_B169_4E78_8663_C241F0A6400A__INCLUDED_)

// BaseNetAgreement.h: interface for the CBaseNetAgreement class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASENETAGREEMENT_H__0D771DDB_704D_4D2F_A62B_A1A3B2BF4DEF__INCLUDED_)
#define AFX_BASENETAGREEMENT_H__0D771DDB_704D_4D2F_A62B_A1A3B2BF4DEF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "NetField.h"
#include "CommField.h"
#include "BaseEquipment.h"
#include "MTJJAEquipment.h"
#include "MTGPSEquipment.h"
#include "TQEquipment.h"
#include "GTQEquipment.h"

/*在处理不同设备的网络请求的时候，处理共同的请求方式*/
class CBaseNetAgreement  
{
public:
	
	void DoQueryGoogleMap(QueryGoogleMap *pDate);
	void DoQueryUserGpsData(CQueryNewestGpsData *pDate);
	void DoGpsDateQuery(CGpsDataQuery *pDate);
	void DoHeartBeat();
	void DoDownLoadUserMapList(CUserMapInfo  *pDate);
	void DoUpDatepsAlarmInfo(CGpsAlarmInfo *pDate);
	void DoUpDateCtrTels(CCtrTels *pDate);
	void DoUpdateAlarmRecvTels(CAlarmRecvTels *pDate);
	void DoDownLaodGpsAlarmInfo(CGpsAlarmInfo  *pDate);
	void DoDownLaodFeeInfo(CFeeInfo *PDate);
	void DoDownLaodAlarmRecvTels(CAlarmRecvTels *pDate);
	void DoCtrTels(CCtrTels *pDate);
	int SendTelMessage(CMessageToClient *pDate);
	void DoOpeateResult(COpeateResult   *pDate);
	void DoMessageToClient(CMessageToClient   *pDate);
	void DoSoftVersionInfo();
	void DoUserPowerInfo(CString szUserId);
	void DoUserQueryEquimentInfo(CUserQueryEquimentInfo *pDate);
	void DoChangeValidDate(CChangeValidDate *pDate);
	void DoChangePwsDate(CChangePwsDate * pDate);
	void DoDataDownLaod(CDownLoadCommand  *pdata);  //处理数据下载的;
	void DoGpsMsg();
	void DoGuardMsg();
	int RestoreUserAllEquiment(CString  szUser);
	int SendToNetMessage(const  BYTE  BMsgType, MIANMSGTYPE  MMsgType,SUBMSGTYPE  SubType,char  *pData,int iDateLen);  //发送网络数据包;
	virtual void DoCommMsg();
	int PackNetMsg(const  BYTE  BMsgType, MIANMSGTYPE  MMsgType,SUBMSGTYPE  SubType,char  *pData,int iDateLen);
		
	void Clear();
	CBaseNetAgreement(int  fd,NetMsgHead *p,int iMsgLen);
	CBaseNetAgreement();

	virtual ~CBaseNetAgreement();
	int m_fd;
protected:
	int RestoreEquimentInfo(CString  EquimentId);
	char  *  m_pBufMsg;  //接收数据的;

	
	NetMsgHead  *m_pThreadMsg;
	int  m_MsgLen;
	//设备的处理
	CBaseEquipment  *m_pBaseEquipment;
	CMTJJAEquipment *m_pMTJJAEquipment;
	CMTGPSEquipment *m_pMTGPSEquipment;
	CTQEquipment    *m_pTQEquipment;
	CGTQEquipment    *m_pGTQEquipment;


private:
	int IsLegalUser(const UserLogin *pUserLogin);  //用户登陆验证;
};

#endif // !defined(AFX_BASENETAGREEMENT_H__0D771DDB_704D_4D2F_A62B_A1A3B2BF4DEF__INCLUDED_)

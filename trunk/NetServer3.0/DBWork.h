// DBWork.h: interface for the CDBWork class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DBWORK_H__47D2F1C6_467A_40D5_87FC_B18769066B33__INCLUDED_)
#define AFX_DBWORK_H__47D2F1C6_467A_40D5_87FC_B18769066B33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ado2.h"
#include "NetField.h"
#include "CommField.h"
#include "GpsFiled.h"
#include "GuardFiled.h"

#include <map>
using namespace std;

class CDBWork  
{
public:
	int GetmaxDianChanRime(CString szEquimentid,CString &szTimelog);
	int UpdataMessageReTime(CString szEquimentId, CString szTimeLog, CString szRTime,CString  szCommdid);
	int insertZXDianchanRun(CString szEquimentId,CString szCommandId,CString  szTimeLog,CString  szText,int  iType);
	int GetDianchanRunInfo(CStringArray  &asInfo);
	int DeleteDiaoDu(CString szEquimentId,CString szKey);
	int GetDiaoDu(TEMP_DioaduMessage  &asDioaduMessage);
	int GetSpeedAlarm(TEMP_SpeedAlarmInfo &asSpeedAlarm);
	int DelSpeedAlarm(CString szKeyid);
	int SetSpeedAlarm(CString szKeyid, int iSpeed1,int iSpeed2);
	int GetEquimentInoFromid(const CString &szKey,CEquimentInfo  &Info);
	int UpDataUserLog(CString  szUserId,CString szTime,CString szIP,int  iType);
	int UpdateEquimentTimeSet(CString szId,CString szText);
	int GetEquimentSetTime(TEMP_EquimentTimeSet  &asTimeSet);
	int GetUserGpsNewestTime(CString szUSerID, CObArray  &aGpsDate);
	int IsHaveGPSDataNewest(CString szEuqimentId,BOOL  &bHave);
	int UpdataGPSNewestData(const CGPSData &pData);
	int CDBWork::CreateTable(CString szTableName);
	int GetGpsNewestTime(CString  szEquientId,CGPSData &data);
	int InsertUserevent(CString szUserID,CString szText,CString  szEquimentId,CString szParam,CString UserIP,int iSOK);
	int UpDateNewestIp(CString  szWeb,CString  szTime,CString szIP);
	int GetNewestIP(CString  &Ip);
	int GetSoVersion(CString  szCompanyId,CString &szComPanyName,int &iVerSion);
	int GetGpsTempReturnId(int  &index);
	int GetUserMap(CString szUserId,CStringArray  &asUserMap);
	int UpDateGpsAlarmInfo(CGpsAlarmInfo  *pInfo);
	int UpdateCtrTels(CString szEquimentId, CString szTels);
	int DeleteCtrTels(CString szEquimentId);
	int DeleteAlarmRecvTels(CString szEquimentId);
	int UpdateAlarmRecvTels(CString szEquimentId,CString  szTels);
	int GetCompanyNoFormEuqimentId(CString szEquimentId,CString &CompanyNo);
	int GetGpsAlarmInfo(CString szEquimentId,TEMP_GpsAlarmInfo &aGpsAlarmInfo);
	int GetFeeInfo(CString  szEquimentId,CFeeInfo  &info);
	int GetAlarmRecvTels(CString szEquimentId,CAlarmRecvTels &info);
	int GetCtrTels(CString szEuqimentId,CCtrTels &CtrTels);
	int InsertTelMessage(CMessageToClient *pDate);
	int InsertOpaterResult(COpeateResult *pDate);
	int GetUserPower(CString szUserId, char *pbuf, int iLen);
	BOOL IsUserSEquiment(const CString &szUserId, const CString &szEquimentId);
	int UpdateUserValiDate(CString szUserId, CString szValiDate);
	BOOL IsUserValiDate(CString szUserId,CString  szValiDate);
	int UpdateUserPws(CString szUserId,CString szNewPws);
	int GetEquimentIdFormNumber(CString   szNum,CString  &szEuqimentId);
	int GetGpsAlarmDealWithInfo(CObArray  &asGpsAlarmDealWithInfo,BOOL  bAll=FALSE);
	int GetGpsData(CString szEuqimentId,CString szBeginTime,CString szEndTime,int Type,TEMP_GPSDATA  &asGpsData);
	int InsertWebReturn(const CWEBSMS &SMS,CString szText, BOOL bSendOK);
	int GetReturnTableMaxID(CString szTableName, int &iMaxID);
	int UpdateGPSTEMPWEB(const CWEBSMS &SMS,BOOL bSendOK);
	int UpdateWebSMS(const CWEBSMS &SMS,BOOL BeOK);
	int GetWebSMS(TEMP_WEBSMS &aWebSMS);
	int UpdateGPSTEMPSMS(const CGuardSMS &SMS,BOOL bSendOK);
	int GetMTGPSCommand_SMSInfo(const  CString & szToEquimentId,const CGuardSMS  & GpsSMS,CGPSCommandSMSInfo  &info);
	int GetTableMaxID(CString  szTableName,int &iMaxID);
	int InsertGuardSMSReturn(const CGuardSMS &SMS,BOOL  bSendOK);
	int UpdateGuardSMS(const CGuardSMS & SMS,BOOL BeOK);
	int GetMTJJACommandInfo(const  CString & szToEquimentId,const CGuardSMS  & guardSMS,CGuardCommandSMSInfo  &info);
	int GetGuardSMS(TEMP_GUARDSMS  & aGuardSMS);
	int GetMTJJACommandInfo(const TEMP_GUARDSMS &asGuardSMS,TEMP_GUARDCOMMANDSMSINFO &  asSMSCommandInfo);
	int GetEquimentUser(CString  szEquimentId,CStringArray &aUsers);
	int UpdataGPSData(CString szTableName,const CGPSData &pData);
	int GetEquimentInfoID(const CString &szKey,CString &szEquimentId, BOOL bMemory);
	int Free_EquimentInfo(CObArray  &asEquimentInfo);
	int GetEquimentInfo(CObArray  & asEquiment);
	int GetEquimentInfo(const CString &szEquimentId,CEquimentInfo &EquiInfo);
	int GetUserAllEquiment(CString  & szUser,CStringArray  &  asAllEquiMent);
	BOOL  IsUserlegal(CString   szUser,CString  szPws);  //得到用户的信息
	int DisConnectDB();
	int ConnectDB(CString  szDataBaseName,CString szUser,CString  szPws);
	static  int DbCount;
	CDBWork();
	virtual ~CDBWork();

	int GetAreaInfo(map<CString, CAreaInfo> &mapAreaInfo);
	int AddEventInfo(CEventInfo info);
	int GetEquipmentType(const CString szEquimentId, int& nCarType, int& nIsRun);

private:
	CADODatabase m_Database;

protected:

};

#endif // !defined(AFX_DBWORK_H__47D2F1C6_467A_40D5_87FC_B18769066B33__INCLUDED_)

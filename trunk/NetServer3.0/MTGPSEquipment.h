// MTGPSEquipment.h: interface for the CMTGPSEquipment class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MTGPSEQUIPMENT_H__98D116ED_3400_418C_9C0D_1A4ECA10C475__INCLUDED_)
#define AFX_MTGPSEQUIPMENT_H__98D116ED_3400_418C_9C0D_1A4ECA10C475__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseEquipment.h"
#include "GpsFiled.h"

typedef  enum  
{
	TYPE_MTGPSNULL=0,
	TYPE_MTGPSREG=1,
	TYPE_MTGPSGPS=2,

}MTGPSDATATYPE;
class CMTGPSEquipment : public CBaseEquipment  
{
public:
	void GetCtrTel(CString   szCtrTel,CStringArray  & aTel);
	BOOL   IsReGpsMessage(CIOCPBuff  *pIOBuf);
	BOOL ReGPSMessage(int fd,struct sockaddr_in addr, const CString &szMsg);
		
	
	BOOL SendCharCommand(CString szEquimentKey,  const CString &szCommand);
	BOOL SendSMSAndWebCommand(CString szEquimentKey, const CString &szCommand,BOOL  bWEB=FALSE);
		
	BOOL SendMTGPSCommand(CString szEquimentKey, const CGpsCommand *pCommand);
		
	MTGPSDATATYPE GetDataType(char *p, int iLen);
	UINT SetState(char *pState);
	BOOL DecGpsData(CIOCPBuff *p,CGPSData &GpsData,CString &szMessage);
	BOOL ParseGpsData(CIOCPBuff  *pIOBuf,CGPSData  &GpsData,CString &szMessage);
	 
	 BOOL   IsRegiste(CIOCPBuff  *pIOBuf);  //是不是注册信息;
	 BOOL   IsDataLegalAndFindPackNum(char *p,int  iLen,int& INumber,int &iEnd);  //过来的数据是不是合法的数据;
	CMTGPSEquipment();
	virtual ~CMTGPSEquipment();
	static   const int iDataNeedLen[5];
private:
BOOL PackCommand(const CGpsCommand *pCommand,char buf[],int iLen);
		

};

#endif // !defined(AFX_MTGPSEQUIPMENT_H__98D116ED_3400_418C_9C0D_1A4ECA10C475__INCLUDED_)

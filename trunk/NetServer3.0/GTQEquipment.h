// GTQEquipment.h: interface for the CGTQEquipment class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GTQEQUIPMENT_H__98D116ED_3400_418C_9C0D_1A4ECA10C475__INCLUDED_)
#define AFX_GTQEQUIPMENT_H__98D116ED_3400_418C_9C0D_1A4ECA10C475__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseEquipment.h"
#include "GpsFiled.h"

typedef  enum  
{
	    GTYPE_TQNULL=0,
		GTYPE_V1=1,
		GTYPE_V4=2,
		GTYPE_I1=3,
		GTYPE_PRESS=4,
		GTYPE_I8=5,
		GTYPE_I2=6,
}GTQDATATYPE;
class CGTQEquipment : public CBaseEquipment  
{
public:
	void SetUDPSata(CString szState, CGPSData &gpsData);
	BOOL SendTQGPSCommandStr(CString szEquimentKey, CString szStr);
		
	COleDateTime  GetYSTime(const CString  &szData);
	COleDateTime  GetNOYSTime(const CString  &szData);
		
		
	byte Getmembyte(byte  &b);
	void SetSata(CString  szState,CGPSData  &gpsData);
	BOOL GetTaxibeWork(CString szKey);
	void SetTaxistate(CString szKey,BOOL  beWork);
	CString GetTaxiData(CString  szKey,CString szStr);
	BOOL PackSMSCommand(char  *pBuf, int  &iLen,CString  szPws2,CString &szMessage);
	BOOL DOTQMessage(char  *pBuf,int  iLen,CString  &szMessage);
	BOOL PackCommand(const CGpsCommand *pCommand,char buf[],int iLen);
		
	BOOL SendTQGPSCommand(CString szEquimentKey, const CGpsCommand *pCommand);
		
	BOOL GetDCBTOASC(char  *  buf,int  iLen,CString  &szStr);
	UINT SetState(char *pState);
	GTQDATATYPE  QueryDataType(char  *p,int ilen,int &SubTypeLen);
	BOOL DecGpsData(CIOCPBuff *p,CGPSData &GpsData,CString &szMessage);
	BOOL ParseGpsData(CIOCPBuff  *pIOBuf,CGPSData  &GpsData,CString &szMessage);
	 
	 BOOL   IsRegiste(CIOCPBuff  *pIOBuf);  //是不是注册信息;
	 BOOL   IsDataLegalAndFindPackNum(char *p,int  iLen,int& INumber,int &iEnd);  //过来的数据是不是合法的数据;
	CGTQEquipment();
	virtual ~CGTQEquipment();
//#ifdef   _TAXI
	CStringArray  m_TaxiWorkArray;
//#endif

};

#endif // !defined(AFX_GTQEQUIPMENT_H__98D116ED_3400_418C_9C0D_1A4ECA10C475__INCLUDED_)

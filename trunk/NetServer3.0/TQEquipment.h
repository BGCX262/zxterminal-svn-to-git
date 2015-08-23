// TQEquipment.h: interface for the CTQEquipment class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TQEQUIPMENT_H__98D116ED_3400_418C_9C0D_1A4ECA10C475__INCLUDED_)
#define AFX_TQEQUIPMENT_H__98D116ED_3400_418C_9C0D_1A4ECA10C475__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseEquipment.h"
#include "GpsFiled.h"

//V1
typedef  struct  tagTQV1Data 
{
	char  Begin;
	char  MakeName[3];	
	char  szKey[11];	
	char  TYPE1[3];	
	
	char  Time[7];	
	char  Able[2];	
	char  Lat[10];	
	char  LatPoint[2];	
	char  Longtinue[11];	
	char  LONGPoint[2];
	char  Speed[7];
	char  Diredtion[4];
	char  Date[7];
	char  state[8];
	char  End;
	
	void  init()
	{
		MakeName[2]=0;
		szKey[10]=0;
		TYPE1[2]=0;
		
		Time[6]=0;
		Able[1]=0;
		Lat[9]=0;
		LatPoint[1]=0;
		Longtinue[10]=0;
		LONGPoint[1]=0;
		Speed[6]=0;
		Diredtion[3]=0;
		Date[6]=0;


	}
	//后，头的数据有变化的部分，用变体来处理
}TQV1Data;


//V4  2
typedef  struct  tagTQV42Data 
{
	char  Begin;
	char  MakeName[3];	
	char  szKey[11];	
	char  TYPE1[3];	
	char  TYPE2[3];
	char  settime[7];
	char  Time[7];	
	char  Able[2];	
	char  Lat[10];	
	char  LatPoint[2];	
	char  Longtinue[11];	
	char  LONGPoint[2];
	char  Speed[7];
	char  Diredtion[4];
	char  Date[7];
	char  state[8];
	char  End;
	void  init()
	{
		MakeName[2]=0;
		szKey[10]=0;
		TYPE1[2]=0;
		TYPE2[2]=0;
		settime[6]=0;
		Time[6]=0;
		Able[1]=0;
		Lat[9]=0;
		LatPoint[1]=0;
		Longtinue[10]=0;
		LONGPoint[1]=0;
		Speed[6]=0;
		Diredtion[3]=0;
		Date[6]=0;
		
	}
	//后，头的数据有变化的部分，用变体来处理
}TQV42Data;

//V4  3
typedef  struct  tagTQV43Data 
{
	char  Begin;
	char  MakeName[3];	
	char  szKey[11];	
	char  TYPE1[3];	
	char  TYPE2[4];
	char  Time[7];	
	char  Able[2];	
	char  Lat[10];	
	char  LatPoint[2];	
	char  Longtinue[11];	
	char  LONGPoint[2];
	char  Speed[7];
	char  Diredtion[4];
	char  Date[7];
	char  state[8];
	char  End;
	void  init()
	{
		MakeName[2]=0;
		szKey[10]=0;
		TYPE1[2]=0;
		TYPE2[3]=0;
		Time[6]=0;
		Able[1]=0;
		Lat[9]=0;
		LatPoint[1]=0;
		Longtinue[10]=0;
		LONGPoint[1]=0;
		Speed[6]=0;
		Diredtion[3]=0;
		Date[6]=0;
		
	}
	//后，头的数据有变化的部分，用变体来处理
}TQV43Data;

//压缩后的数据;
typedef  struct  tagTQPREDATA
{
	char  Begin;
	char  MakeName[3];	
	char  szKey[11];	
	char  TYPE1[3];	
	char  TYPE2[4];
	char  Time[7];	
	char  Able[2];	
	char  Lat[10];	
	char  LatPoint[2];	
	char  Longtinue[11];	
	char  LONGPoint[2];
	char  Speed[7];
	char  Diredtion[4];
	char  Date[7];
	char  state[8];
	char  End;
	void  init()
	{
		MakeName[2]=0;
		szKey[10]=0;
		TYPE1[2]=0;
		TYPE2[3]=0;
		Time[6]=0;
		Able[1]=0;
		Lat[9]=0;
		LatPoint[1]=0;
		Longtinue[10]=0;
		LONGPoint[1]=0;
		Speed[6]=0;
		Diredtion[3]=0;
		Date[6]=0;
		
	}
	//后，头的数据有变化的部分，用变体来处理
}TQPREDATA;

//短信息的头文件
//*XX,YYYYYYYYYY,I1,HHMMSS,1,Code,Info_lenth,Information#
typedef  struct  tagTQMESSAGE
{
	char  Begin;
	char  MakeName[3];	
	char  szKey[11];	
	char  TYPE[3];		
	char  Time[7];	
	char  Fix[2];	  //1
	char  Code[2];	
	void  init()
	{
		MakeName[2]=0;
		szKey[10]=0;
		TYPE[2]=0;	
		Time[6]=0;
		Fix[1]=0;
		Code[1]=0;		
	}
}TQMESSAGE;

class   BCDCHAR
{
public:
	unsigned short    B1 :4;
	unsigned  short    B2 :4;
};

typedef  enum  
{
	    TYPE_TQNULL=0,
		TYPE_V1=1,
		TYPE_V4=2,
		TYPE_I1=3,
		TYPE_PRESS=4,
		TYPE_I8=5,
}TQDATATYPE;
class CTQEquipment : public CBaseEquipment  
{
public:
	COleDateTime GetYSTime(const CString  &szData);
	COleDateTime  GetNOYSTime(const CString  &szData);
	BOOL SendTQGPSCommandStr(CString szEquimentKey,CString szStr);
	BOOL GetTaxibeWork(CString szKey);
	void SetTaxistate(CString szKey,BOOL  beWork);
	CString GetTaxiData(CString  szKey,CString szStr);
	BOOL PackSMSCommand(char  *pBuf, int  &iLen,CString &szMessage);
	BOOL DOTQMessage(char  *pBuf,int  iLen,CString  &szMessage);
	BOOL PackCommand(const CGpsCommand *pCommand,char buf[],int iLen);
		
	BOOL SendTQGPSCommand(CString szEquimentKey, const CGpsCommand *pCommand);
		
	BOOL GetDCBTOASC(char  *  buf,int  iLen,CString  &szStr);
	UINT SetState(char *pState);
	TQDATATYPE  QueryDataType(char  *p,int ilen,int &SubTypeLen);
	BOOL DecGpsData(CIOCPBuff *p,CGPSData &GpsData,CString &szMessage);
	BOOL ParseGpsData(CIOCPBuff  *pIOBuf,CGPSData  &GpsData,CString &szMessage);
	 
	 BOOL   IsRegiste(CIOCPBuff  *pIOBuf);  //是不是注册信息;
	 BOOL   IsDataLegalAndFindPackNum(char *p,int  iLen,int& INumber,int &iEnd);  //过来的数据是不是合法的数据;
	CTQEquipment();
	virtual ~CTQEquipment();
//#ifdef   _TAXI
	CStringArray  m_TaxiWorkArray;
//#endif

};

#endif // !defined(AFX_TQEQUIPMENT_H__98D116ED_3400_418C_9C0D_1A4ECA10C475__INCLUDED_)

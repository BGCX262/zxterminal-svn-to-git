// BaseEquipment.cpp: implementation of the CBaseEquipment class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseEquipment.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEquimentList  CBaseEquipment::m_EquimenList;
CBaseEquipment::CBaseEquipment()
{


}

CBaseEquipment::~CBaseEquipment()
{


}
double CBaseEquipment::DecLONG(char *pLONG)
{
	double   Dlong=atof(pLONG);
	int  ilong=(int)(Dlong/100);
	return ilong+(Dlong-ilong*100)/60;	  
}

double CBaseEquipment::DecLat(char *pLat)
{
	double   DLat=atof(pLat);
	int  iLat=(int)(DLat/100);
	return iLat+(DLat-iLat*100)/60;

}
BOOL CBaseEquipment::ParseGpsData(CIOCPBuff  *pIOBuf,CGPSData  &GpsData,CString &szMessage)
{
	return FALSE;
}
BOOL CBaseEquipment::ParseGuardData(CIOCPBuff  *pIOBuf,CGuardData  &GuardData)
{
	return FALSE;
}
BOOL  CBaseEquipment::IsRegiste(CIOCPBuff  *pIOBuf)
{
	return  FALSE;
}
BOOL   CBaseEquipment::IsDataLegalAndFindPackNum(char *p,int  iLen,int& INumber,int &iEnd)
{
	return FALSE;
}

BOOL CBaseEquipment::SendCommand(CString szEquimentKey, const CGuardCommand *pCommand)
{

	return FALSE;
}

BOOL   CBaseEquipment::ParseLatAndLong(CString szData, BOOL bLong,double  &dLongOrLatValue )
{
	if(szData.IsEmpty())
		return  FALSE;
	if(bLong)
	{//¾­¶È11157.9107
	  double   dValue=atof(szData.GetBuffer(0));
	  if(dValue<=0)
		  return FALSE;
	  int  iLong=( int (dValue/100) %1000);
	  if(dValue<iLong)
		  return FALSE;
	  double dLong=dValue-iLong*100;
	  double  dRet=iLong+dLong/60;
	  dLongOrLatValue=dRet;
	  if(dLongOrLatValue>150)
		  return FALSE;
	}
	if(!bLong)
	{//Î³¶È2152.3654

		double   dValue=atof(szData.GetBuffer(0));
		if(dValue<=0)
			return FALSE;
		int  iLat=( int (dValue/100) %100);
		if(dValue<iLat)
			return FALSE;
		double dLat=dValue-iLat*100;
		double  dRet=iLat+dLat/60;
		dLongOrLatValue=dRet;
		if(dLongOrLatValue>90)
			return FALSE;
	}
	return TRUE;

}

// BaseEquipment.h: interface for the CBaseEquipment class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASEEQUIPMENT_H__604BDF26_BFDE_47FA_8667_1D5BDE2C7018__INCLUDED_)
#define AFX_BASEEQUIPMENT_H__604BDF26_BFDE_47FA_8667_1D5BDE2C7018__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "EquimentList.h"
#include "IOCPBuff.h"
#include "GpsFiled.h"
#include "GuardFiled.h"
/*所有设备的基类*/
class CBaseEquipment  
{
public:
    BOOL   ParseLatAndLong(CString szData, BOOL bLong,double  &dLongOrLatValue );
	virtual BOOL SendCommand(CString   szEquimentKey,const CGuardCommand *pCommand);
	double DecLat(char *pLat);
	double DecLONG(char *pLONG);
	virtual BOOL ParseGpsData(CIOCPBuff  *pIOBuf,CGPSData  &GpsData,CString &szMessage); //解析数据;
	virtual BOOL ParseGuardData(CIOCPBuff  *pIOBuf,CGuardData  &GuardData); //解析数据;
	
	virtual BOOL   IsRegiste(CIOCPBuff  *pIOBuf);  //是不是注册信息;
	virtual BOOL   IsDataLegalAndFindPackNum(char *p,int  iLen,int& INumber,int &iEnd);  //过来的数据是不是合法的数据;
	
	CBaseEquipment();
	virtual ~CBaseEquipment();	
protected:
	static CEquimentList  m_EquimenList;
	
	

};

#endif // !defined(AFX_BASEEQUIPMENT_H__604BDF26_BFDE_47FA_8667_1D5BDE2C7018__INCLUDED_)






















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
/*�����豸�Ļ���*/
class CBaseEquipment  
{
public:
    BOOL   ParseLatAndLong(CString szData, BOOL bLong,double  &dLongOrLatValue );
	virtual BOOL SendCommand(CString   szEquimentKey,const CGuardCommand *pCommand);
	double DecLat(char *pLat);
	double DecLONG(char *pLONG);
	virtual BOOL ParseGpsData(CIOCPBuff  *pIOBuf,CGPSData  &GpsData,CString &szMessage); //��������;
	virtual BOOL ParseGuardData(CIOCPBuff  *pIOBuf,CGuardData  &GuardData); //��������;
	
	virtual BOOL   IsRegiste(CIOCPBuff  *pIOBuf);  //�ǲ���ע����Ϣ;
	virtual BOOL   IsDataLegalAndFindPackNum(char *p,int  iLen,int& INumber,int &iEnd);  //�����������ǲ��ǺϷ�������;
	
	CBaseEquipment();
	virtual ~CBaseEquipment();	
protected:
	static CEquimentList  m_EquimenList;
	
	

};

#endif // !defined(AFX_BASEEQUIPMENT_H__604BDF26_BFDE_47FA_8667_1D5BDE2C7018__INCLUDED_)






















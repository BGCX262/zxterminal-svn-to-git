// EquipmentManager1.cpp: implementation of the CEquipmentManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "EquipmentManager.h"
#include "SystemInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEquipmentManager::CEquipmentManager()
{

	m_pBaseEquipment=NULL;
	m_pMTJJAEquipment=NULL;
}

CEquipmentManager::~CEquipmentManager()
{

}

BOOL CEquipmentManager::DoEquipmentData(CIOCPBuff  *pIOBuf)
{
	if(!pIOBuf)
		return  FALSE;
	switch(pIOBuf->m_iEuqType  ) 
	{
	case  EQUIPMENT_MTJJA :
		m_pBaseEquipment=(CMTJJAEquipment *)m_pMTJJAEquipment;
		break;

	default:
		ASSERT(FALSE);
		return FALSE;
		break;
	}
	ASSERT(FALSE);
	return  TRUE;

}

BOOL CEquipmentManager::Start()
{
	//´´½¨
//	m_pBaseEquipment=NULL;
		//	if(!m_pMTJJAEquipment)
		//		delete   m_pMTJJAEquipment;
		//	m_pMTJJAEquipment=NULL;
		//	m_pMTJJAEquipment =new   CMTJJAEquipment;
		//	if(!m_pMTJJAEquipment)
		//		return  FALSE;  
	return  FALSE;
		
}






















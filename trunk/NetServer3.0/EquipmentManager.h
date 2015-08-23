// EquipmentManager1.h: interface for the CEquipmentManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EQUIPMENTMANAGER1_H__675F7ED2_0FDB_49F2_B629_781DC207A68A__INCLUDED_)
#define AFX_EQUIPMENTMANAGER1_H__675F7ED2_0FDB_49F2_B629_781DC207A68A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "IOCPBuff.h"
#include "BaseEquipment.h"
#include "MTJJAEquipment.h"

/*这个类作为一个容器，把所有的设备协议相关的东西包含在里头*/
class CEquipmentManager  
{
public:
	BOOL Start();
	BOOL DoEquipmentData(CIOCPBuff  *pIOBuf);  //处理数据;
	CEquipmentManager();
	virtual ~CEquipmentManager();
private:
	CBaseEquipment  *m_pBaseEquipment;
	CMTJJAEquipment *m_pMTJJAEquipment;

};

#endif // !defined(AFX_EQUIPMENTMANAGER1_H__675F7ED2_0FDB_49F2_B629_781DC207A68A__INCLUDED_)

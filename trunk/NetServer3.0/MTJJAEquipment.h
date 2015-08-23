// MTJJAEquipment.h: interface for the CMTJJAEquipment class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MTJJAEQUIPMENT_H__295B2A8D_CCFA_4F7E_A4C4_6E87E4B49386__INCLUDED_)
#define AFX_MTJJAEQUIPMENT_H__295B2A8D_CCFA_4F7E_A4C4_6E87E4B49386__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseEquipment.h"
#include "IOCPBuff.h"
#include <afxsock.h>

/*������ӼҼҰ��Ľ�����*/
typedef  enum  
{
	    TYPE_MTJJANULL=0,
	    TYPE_ROK=1,
		TYPE_REG=2,
		TYPE_A=3,
		TYPE_ERR=4,
		
}MTJJADATATYPE;


class CMTJJAEquipment : public CBaseEquipment  
{
public:
	BOOL SendWebCommand(CString szEquimentKey, const CString &szCommand);
	BOOL SendSMSAndWebCommand(CString szEquimentKey, const  CString &szCommand,BOOL  bweb=FALSE);
	BOOL ReMessage(CString szEquimentKey, const CString  &szMsg);
	BOOL SendCommand(CString   szEquimentKey,const CGuardCommand *pCommand);
	MTJJADATATYPE GetDataType(char  *p,int iLen);  //�õ���ǰ���ݵ�����;
	BOOL DecGuardData(CIOCPBuff *p, MTJJADATATYPE iDataType,CGuardData &GuardData);
	BOOL  ParseGuardData(CIOCPBuff  *pIOBuf,CGuardData  &GuardData);
	virtual BOOL   IsRegiste(CIOCPBuff  *pIOBuf);  //�ǲ���ע����Ϣ;
	virtual BOOL   IsDataLegalAndFindPackNum(char *p,int  iLen,int& INumber,int &iEnd);;  //�����������ǲ��ǺϷ�������;
	CMTJJAEquipment();
	virtual ~CMTJJAEquipment();
    static   const int iDataNeedLen[5];

protected:
	BOOL PackCommand(const CGuardCommand *pCommand,char  bufcommand[],int iBufLen);
  //  CAsyncSocket  m_SendSock;
//	int  Sendfd;
};

#endif // !defined(AFX_MTJJAEQUIPMENT_H__295B2A8D_CCFA_4F7E_A4C4_6E87E4B49386__INCLUDED_)























/*�豸�����˰�ȫ����������ݶ���*/
#ifndef   _GUARDFIELD_H_
#define   _GUARDFIELD_H_
#include <afxtempl.h>
class   CGuardData
{
public:
	char  EquimentId[15];
    char  szTime[20];
	BYTE  DataType;   // 0:һ����Ϣ��1���ظ���Ϣ��2������Ϣ;
	int   ProbeType ; //̽���;  -1:��Ч  0��һ�������;
	UINT  state;      //״̬;
	CGuardData()
	{
		ZeroMemory(EquimentId,15);
		ProbeType=0;
		state=0;
		DataType=0;
		ZeroMemory(szTime,20);
	}		
};
//����͵Ľṹ;
class  CGuardCommand
{
public:
	char EquimentId[15];  //�豸��id;
	int  iCOmmandID;   //�����id�����ʱ����;
	char PWS[20];//���͵�����pws��
	char comandText[50];//���������;
	char Param[50]; //����
	CGuardCommand()
	{
		memset(EquimentId,0,15);
		iCOmmandID=0;
		memset(PWS,0,20);
		memset(comandText,0,50);
		memset(Param,0,50);
	}
};

//����Ϣ�������������ʽ
class  CGuardCommandSMSInfo
{
	public:
	CString  szConent;
	CString  szInfo;
	CString  szToSimid;
	CString  szSIMID;

};
typedef   CArray<CGuardCommandSMSInfo,CGuardCommandSMSInfo&>TEMP_GUARDCOMMANDSMSINFO;


//����Ϣ��ؿ��������ʽ
class  CGPSCommandSMSInfo
{
public:
	CString  szToEQUIMENTID;
	CString  szCommandText;
	CString  szFromSimid;
};
typedef   CArray<CGPSCommandSMSInfo,CGPSCommandSMSInfo&>TEMP_GPSCOMMANDSMSINFO;

class CGuardSMS
{
public:
	CString  szSIMID;
	CString  szSMS;
	CString  szTime;
	CString  szRemark;	
};
typedef   CArray<CGuardSMS,CGuardSMS&>TEMP_GUARDSMS;

//web ��������Ľṹ
class CWEBSMS
{
public:
	CString  szSENDSIMID;
	CString  szMTWEBCOMMANDCONTENT;
	CString  szTIMELOG;
	int  ISSENDSUCCESS;	
	int  SOCKETINDEX;
};
typedef   CArray<CWEBSMS,CWEBSMS&>TEMP_WEBSMS;


#endif

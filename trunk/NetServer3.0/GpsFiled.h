
/*�������gps���������ݽṹ*/
#ifndef    _GPSFILED_H_
#define    _GPSFILED_H_

#include <afxtempl.h>

#define   DEF_XMLMAX   200    //xml�ļ��������
//���ӹ̶���Ϣ
#define   GPS_DATA_INLEGAL        9997    //���Ϸ���gps����
#define   USER_DEFINED      99   //�Զ��������ID
//��Ӧ����Ϣ;
#define   GPS_SETTIMETVOK           1001    //����ʱ�����ɹ�;
#define   GPS_SETTIMETVERROR         2001    //����ʱ����ʧ��;

#define   GPS_LOCKCAROK           1002    //�����ɹ�
#define   GPS_LOCKCARERROR           2002    //����ʧ��

#define   GPS_SETREGIONOK         1003   //���򱨾����óɹ�;
#define   GPS_SETREGIONERROR      2003   //���򱨾�����ʧ��;

#define   GPS_SETSPEEDOK          1004   //�ٶȱ������óɹ�;
#define   GPS_SETSPEEDERROR          2004   //�ٶȱ�������ʧ��;

#define   GPS_SETLISTENOK         1005  //���ü����绰�ɹ�
#define   GPS_SETLISTENERROR       2005  //���ü����绰ʧ��

#define   GPS_SETTELOK             1006  //���õ绰���Ƴɹ�;
#define   GPS_SETTELERROR          2006  //���õ绰����ʧ��;

#define    GPS_SETRELIEVEOK        1007  //��������ɹ�
#define    GPS_SETRELIEVEERROR      2007  //�������ʧ��;

#define    GPS_SENDMESSAGEOK        1008  //����Ϣ���ͳɹ�
#define    GPS_SENDMESSAGEERROR      2008  //����Ϣ����ʧ��;


//������Ϣ;
#define    GPS_LOWSPEEDALARM        9001  //���ٶȱ�����
#define    GPS_HIGHSPEEDALARM       9002  //���ٶȱ�����

#define    GPS_ROBALARM             9003//�������;
#define    GPS_REGIONALARM          9004//�������򱨾�;

#define    GPS_NOelectricity          9005//����
#define    GPS_NOGPSINFO          9006//gps��·

//״̬��Ϣ 


#define    GPS_STATONE             3001  //��������չ1  //������������ؿ�״̬
#define    GPS_STATTWO             3002  //��������չ2


#define    GPS_STATTHREE             3003  //��������չ3
#define    GPS_STATFOR             3004  //��������չ4
#define    GPS_STATTFIVE            3005  //��������չ5

#define    GPS_DOOR          3006//����
#define    GPS_ACC         3007//ACC
#define    GPS_POWER         3009//������

class  CGPSData 
{
public:
	char   szEquimentID[15]; //�豸��id;
	double  fLongitude;  //����
	double  fLatitude;   //γ��
	float  fSpeed;     //�ٶ�
	short  iDirection; //����
	UINT   iState;     //״̬
	char   Time[20];
	int    iFlag;     //������Ч��
	CGPSData()
	{
		memset(szEquimentID,0,15);
		memset(Time,0,20);
		fLongitude=0.0;
		fLatitude=0.0;
		fSpeed=0;
		iDirection=0;
		iState=0;
		iFlag=0;
	}
};
typedef   CArray<CGPSData,CGPSData&> TEMP_GPSDATA;
//����Ϣ
class  CGPSMESSAGE 
{
public:
	char   szEquimentID[15]; //�豸��id;
	char    szMsg[200]; //����Ϣ;
	char    szTime[20]; //ʱ��;
	CGPSMESSAGE()
	{
		memset(szEquimentID,0,15);
		memset(szMsg,0,200);
		memset(szTime,0,20);
	}
};


//����͵Ľṹ;
class  CGpsCommand
{
public:
	char EquimentId[15];  //�豸��id;
	int  iCOmmandID;   //�����id�����ʱ����;
	char PWS[20];//���͵�����pws��
	char Param[200]; //����
	CGpsCommand()
	{
		memset(EquimentId,0,15);
		iCOmmandID=0;
		memset(PWS,0,20);
		memset(Param,0,200);
	}
};
//GPS �ı���������Ϣ
class  CGpsAlarmDealWithInfo
{
public:
	char   EQUIMENTID[15];
	int    STATE;   //״̬
	char   DESCRIPTION[50];  //����
	int    GRADE ;// �ȼ�
	int    ALARMNUMBER;  //������;
	char   DEALWITH[50];//����ʽ;
	int    BESEND ;  //�ǲ��Ƿ��͵��ͻ��ֻ�
	char  TELS[200]; //���͵��ֻ���Ϣ;
	CGpsAlarmDealWithInfo()
	{
		memset(EQUIMENTID,0,15);
		memset(DESCRIPTION,0,50);
		memset(DEALWITH,0,50);
		BESEND=1;
		memset(TELS,0,200);
	}
	
};
//GPS�豸�ı�������
class  CGpsAlarmInfo
{
public:
	char  EuqimentId[15];
	int   AlarmState;
	char  info[50];
	int   Grade;
	int   AlarmNumber;
	char  DealWith[50];
	int    BESEND ;  //�ǲ��Ƿ��͵��ͻ��ֻ�
	CGpsAlarmInfo()
	{
		memset(EuqimentId,0,15);
		memset(info,0,50);
		memset(DealWith,0,50);
		Grade=0;
		AlarmNumber=0;
		AlarmState=0;		
	}
	
};
typedef   CArray<CGpsAlarmInfo,CGpsAlarmInfo&> TEMP_GpsAlarmInfo;

//���������ʱ��;
class  CGpsDataQuery
{
public:
	char  szEquimentID[15];
	char  szTime[20];
	BYTE  itype;
	CGpsDataQuery()
	{
		memset(szEquimentID,0,15);
		memset(szTime,0,20);
		itype=0;  //������������;
		
	}
};

//�����û��豸����������

class  CQueryNewestGpsData
{
public:
	char  szUsrID[30];
	
	CQueryNewestGpsData()
	{
		memset(szUsrID,0,30);	
	}
};
//end;


//�豸��ʱ������;
class  CEquimentTimeSet
{
public:
    CString  szEquimentId;
	CString  szStr;
};
typedef   CArray<CEquimentTimeSet,CEquimentTimeSet&> TEMP_EquimentTimeSet;



class   CSpeedAlarmInfo
{
public:
	CString  szKeyId;
	int   iLSpeed; 
	int   iHSpeed;  
};
typedef   CArray<CSpeedAlarmInfo,CSpeedAlarmInfo&> TEMP_SpeedAlarmInfo;
//end;

class   CPicMessage
{
public:
	CString   szEquimentid;
	CString   szMessage;
	CString    szTime;
};
typedef   CArray<CPicMessage,CPicMessage&> TEMP_PicMessage;


class   CDioaduMessage
{
public:
	CString   szEquimentid;
	CString   szKey;
	CString    szMessage;
};
typedef   CArray<CDioaduMessage,CDioaduMessage&> TEMP_DioaduMessage;


#endif






















/*设备定义了安全防盗类的数据定义*/
#ifndef   _GUARDFIELD_H_
#define   _GUARDFIELD_H_
#include <afxtempl.h>
class   CGuardData
{
public:
	char  EquimentId[15];
    char  szTime[20];
	BYTE  DataType;   // 0:一般信息；1；回复信息；2报警信息;
	int   ProbeType ; //探针号;  -1:无效  0：一般的数据;
	UINT  state;      //状态;
	CGuardData()
	{
		ZeroMemory(EquimentId,15);
		ProbeType=0;
		state=0;
		DataType=0;
		ZeroMemory(szTime,20);
	}		
};
//命令发送的结构;
class  CGuardCommand
{
public:
	char EquimentId[15];  //设备的id;
	int  iCOmmandID;   //命令的id这个暂时保留;
	char PWS[20];//发送的命令pws；
	char comandText[50];//命令的内容;
	char Param[50]; //参数
	CGuardCommand()
	{
		memset(EquimentId,0,15);
		iCOmmandID=0;
		memset(PWS,0,20);
		memset(comandText,0,50);
		memset(Param,0,50);
	}
};

//短信息安防控制命令格式
class  CGuardCommandSMSInfo
{
	public:
	CString  szConent;
	CString  szInfo;
	CString  szToSimid;
	CString  szSIMID;

};
typedef   CArray<CGuardCommandSMSInfo,CGuardCommandSMSInfo&>TEMP_GUARDCOMMANDSMSINFO;


//短信息监控控制命令格式
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

//web 控制命令的结构
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

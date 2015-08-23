
/*定义关于gps的所有数据结构*/
#ifndef    _GPSFILED_H_
#define    _GPSFILED_H_

#include <afxtempl.h>

#define   DEF_XMLMAX   200    //xml文件的最大行
//增加固定信息
#define   GPS_DATA_INLEGAL        9997    //不合法的gps数据
#define   USER_DEFINED      99   //自定义的命令ID
//回应的信息;
#define   GPS_SETTIMETVOK           1001    //设置时间间隔成功;
#define   GPS_SETTIMETVERROR         2001    //设置时间间隔失败;

#define   GPS_LOCKCAROK           1002    //锁车成功
#define   GPS_LOCKCARERROR           2002    //锁车失败

#define   GPS_SETREGIONOK         1003   //区域报警设置成功;
#define   GPS_SETREGIONERROR      2003   //区域报警设置失败;

#define   GPS_SETSPEEDOK          1004   //速度报警设置成功;
#define   GPS_SETSPEEDERROR          2004   //速度报警设置失败;

#define   GPS_SETLISTENOK         1005  //设置监听电话成功
#define   GPS_SETLISTENERROR       2005  //设置监听电话失败

#define   GPS_SETTELOK             1006  //设置电话限制成功;
#define   GPS_SETTELERROR          2006  //设置电话限制失败;

#define    GPS_SETRELIEVEOK        1007  //解除报警成功
#define    GPS_SETRELIEVEERROR      2007  //解除报警失败;

#define    GPS_SENDMESSAGEOK        1008  //短信息发送成功
#define    GPS_SENDMESSAGEERROR      2008  //短信息发送失败;


//报警信息;
#define    GPS_LOWSPEEDALARM        9001  //低速度报警；
#define    GPS_HIGHSPEEDALARM       9002  //高速度报警；

#define    GPS_ROBALARM             9003//紧急求救;
#define    GPS_REGIONALARM          9004//超出区域报警;

#define    GPS_NOelectricity          9005//掉电
#define    GPS_NOGPSINFO          9006//gps开路

//状态信息 


#define    GPS_STATONE             3001  //用来做扩展1  //用来做出租的载可状态
#define    GPS_STATTWO             3002  //用来做扩展2


#define    GPS_STATTHREE             3003  //用来做扩展3
#define    GPS_STATFOR             3004  //用来做扩展4
#define    GPS_STATTFIVE            3005  //用来做扩展5

#define    GPS_DOOR          3006//车门
#define    GPS_ACC         3007//ACC
#define    GPS_POWER         3009//发动机

class  CGPSData 
{
public:
	char   szEquimentID[15]; //设备的id;
	double  fLongitude;  //经度
	double  fLatitude;   //纬度
	float  fSpeed;     //速度
	short  iDirection; //方向
	UINT   iState;     //状态
	char   Time[20];
	int    iFlag;     //卫星有效性
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
//短信息
class  CGPSMESSAGE 
{
public:
	char   szEquimentID[15]; //设备的id;
	char    szMsg[200]; //短信息;
	char    szTime[20]; //时间;
	CGPSMESSAGE()
	{
		memset(szEquimentID,0,15);
		memset(szMsg,0,200);
		memset(szTime,0,20);
	}
};


//命令发送的结构;
class  CGpsCommand
{
public:
	char EquimentId[15];  //设备的id;
	int  iCOmmandID;   //命令的id这个暂时保留;
	char PWS[20];//发送的命令pws；
	char Param[200]; //参数
	CGpsCommand()
	{
		memset(EquimentId,0,15);
		iCOmmandID=0;
		memset(PWS,0,20);
		memset(Param,0,200);
	}
};
//GPS 的报警处理信息
class  CGpsAlarmDealWithInfo
{
public:
	char   EQUIMENTID[15];
	int    STATE;   //状态
	char   DESCRIPTION[50];  //描述
	int    GRADE ;// 等级
	int    ALARMNUMBER;  //报警号;
	char   DEALWITH[50];//处理方式;
	int    BESEND ;  //是不是发送到客户手机
	char  TELS[200]; //发送的手机信息;
	CGpsAlarmDealWithInfo()
	{
		memset(EQUIMENTID,0,15);
		memset(DESCRIPTION,0,50);
		memset(DEALWITH,0,50);
		BESEND=1;
		memset(TELS,0,200);
	}
	
};
//GPS设备的报警描述
class  CGpsAlarmInfo
{
public:
	char  EuqimentId[15];
	int   AlarmState;
	char  info[50];
	int   Grade;
	int   AlarmNumber;
	char  DealWith[50];
	int    BESEND ;  //是不是发送到客户手机
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

//数据请求的时间;
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
		itype=0;  //请求最新数据;
		
	}
};

//请求用户设备的最新数据

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


//设备的时间设置;
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





















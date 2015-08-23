
/*定义了和设备无关的数据结构*/
#ifndef    _COMMFIELD_H_
#define    _COMMFIELD_H_
//用户信息;
class  CUserInfo
{
	char szUserId[30];
	char  szPws[30];
	char  szUserUnmber[10];
	char  sztel[50];
	char  szParentcompany[50];
	char  szaddr[50];
	char  szcompany[50];
    CUserInfo()
	{
		ZeroMemory(szUserId,30);
		ZeroMemory(szPws,30);
		ZeroMemory(szUserUnmber,10);
		ZeroMemory(sztel,50);
		ZeroMemory(szParentcompany,50);
		ZeroMemory(szaddr,50);
		ZeroMemory(szcompany,50);
	}

};

class CUserAllEquiment
{
public:
	char  *pAllEqui;
	int  iLen;
	CUserAllEquiment(CString  szAllEqui)
	{
		pAllEqui=NULL;
		iLen=0;		
		if(!szAllEqui.IsEmpty())
		{
			iLen=szAllEqui.GetLength();
            pAllEqui=new char [iLen];
			if(!pAllEqui)
				iLen=0;
			else
			{//有数据的时候进行处理;
				ZeroMemory(pAllEqui,iLen);
			//	sprintf(pAllEqui,"%s",szAllEqui);
				memcpy(pAllEqui,szAllEqui.GetBuffer(0),iLen);
				pAllEqui[--iLen]=0;
			}			
		}		
	}
	~CUserAllEquiment()
	{
		if(pAllEqui)
			delete []pAllEqui;
		iLen=0;
	}
};
//请求设备的信息;
class  CQueryEquimnetInfo
{
public:
	char m_EquimentId[15];
	int  iLen;
	CQueryEquimnetInfo(CString szEquimentId)
	{
		ZeroMemory(m_EquimentId,15);
		sprintf(m_EquimentId,"%s",szEquimentId);
		m_EquimentId[szEquimentId.GetLength()]=0;
		iLen=szEquimentId.GetLength();
	}
};
//设备的信息
class  CEquimentInfo
{
public:
	char EquimentId[15];
	char EquimentKey[15];
	UINT  EquimentMainType;
	UINT  EquimentSubType;
	char  EQUIMENTNUMBER[30];
	char  INDUSTRYNUMBER[30];
	char  OWNERNAME[30];
	char  USERNAME1[30];
	char  USERNAME2[30];
	char  PHONE[30];
	char  REMARK[200];
	CEquimentInfo(const CEquimentInfo &EquiInfo)
	{
		ZeroMemory(EquimentId,15);
		ZeroMemory(EquimentKey,15);
		ZeroMemory(EQUIMENTNUMBER,30);
		ZeroMemory(INDUSTRYNUMBER,30);
		ZeroMemory(OWNERNAME,30);
		ZeroMemory(USERNAME1,30);
		ZeroMemory(USERNAME2,30);
		ZeroMemory(PHONE,30);
		ZeroMemory(REMARK,30);
		EquimentMainType=EquiInfo.EquimentMainType;
		EquimentSubType=EquiInfo.EquimentSubType;
		memcpy(EquimentId,EquiInfo.EquimentId,15);
		memcpy(EquimentKey,EquiInfo.EquimentKey,15);
		memcpy(EQUIMENTNUMBER,EquiInfo.EQUIMENTNUMBER,30);
		memcpy(INDUSTRYNUMBER,EquiInfo.INDUSTRYNUMBER,30);
		memcpy(OWNERNAME,EquiInfo.OWNERNAME,30);
		memcpy(USERNAME1,EquiInfo.USERNAME1,30);
		memcpy(USERNAME2,EquiInfo.USERNAME2,30);
		memcpy(PHONE,EquiInfo.PHONE,30);
		memcpy(REMARK,EquiInfo.REMARK,200);
	}
	CEquimentInfo()
	{
		ZeroMemory(EquimentId,15);
		ZeroMemory(EquimentKey,15);
		ZeroMemory(EQUIMENTNUMBER,30);
		ZeroMemory(INDUSTRYNUMBER,30);
		ZeroMemory(OWNERNAME,30);
		ZeroMemory(USERNAME1,30);
		ZeroMemory(USERNAME2,30);
		ZeroMemory(PHONE,30);
		ZeroMemory(REMARK,200);
	}
	CEquimentInfo (CString szEID)
	{
		ZeroMemory(EquimentId,15);
		sprintf(EquimentId,"%s",szEID);
		EquimentId[szEID.GetLength()]=0;
		ZeroMemory(EquimentKey,15);
		ZeroMemory(EQUIMENTNUMBER,30);
		ZeroMemory(INDUSTRYNUMBER,30);
		ZeroMemory(OWNERNAME,30);
		ZeroMemory(USERNAME1,30);
		ZeroMemory(USERNAME2,30);
		ZeroMemory(PHONE,30);
		ZeroMemory(REMARK,200);

	}
	CEquimentInfo  & operator=(const CEquimentInfo &EquiInfo)
	{
		ZeroMemory(EquimentId,15);
		ZeroMemory(EquimentKey,15);
		EquimentMainType=EquiInfo.EquimentMainType;
		EquimentSubType=EquiInfo.EquimentSubType;
		memcpy(EquimentId,EquiInfo.EquimentId,15);
		memcpy(EquimentKey,EquiInfo.EquimentKey,15);		
		return  *this;
	}
	

};
//发送到客户端的回应信息;
class   CResponCommandInfo
{
public:
	char EquimnetID[15];//暂时保留;
	char commandText[100];//命令的内容，用于命令员包返回;
	CResponCommandInfo()
	{
		memset(EquimnetID,0,15);
		memset(commandText,0,100);

	}

};

//设备的用户
class  CEquimentUser
{
public:
	CString  m_szEquimentId;
	CStringArray  m_asUser;
	CEquimentUser()
	{
		m_szEquimentId.Empty();
		m_asUser.RemoveAll();
		m_asUser.SetSize(0);
	}
	CEquimentUser(CString szEquimentId)
	{
		m_szEquimentId=szEquimentId;
		m_asUser.RemoveAll();
		m_asUser.SetSize(0);
	}
};
//密码更改
class  CChangePwsDate
{
public:
	char   szUserId[30];
	char   szOldPws[30];
	char   szNewPws[30];
	char   sztwoPws[30];
	CChangePwsDate()
	{
		memset(szUserId,0,30);
		memset(szOldPws,0,30);
		memset(szNewPws,0,30);
		memset(sztwoPws,0,30);
	}
	
};
//验证号更改
class  CChangeValidDate
{
public:
	char   szUserId[30];
	char   szOldValidDate[30];
	char   szNewValidDate[30];
	char   sztwoValidDate[30];
	CChangeValidDate()
	{
		memset(szUserId,0,30);
		memset(szOldValidDate,0,30);
		memset(szNewValidDate,0,30);
		memset(sztwoValidDate,0,30);
	}
	
};
//客户咨询
class  CUserQueryEquimentInfo
{
public:
	char   szUserId[30];
	char   szEuqimentId[15];
	char   szUserValidDate[30];
	
	CUserQueryEquimentInfo()
	{
		memset(szUserId,0,30);
		memset(szEuqimentId,0,15);
		memset(szUserValidDate,0,30);
	}	
};

//权限信息
typedef struct  tagUserpower
{
	char szUserId[30];
    char szPower[300];
	BYTE  beUsed;
	tagUserpower()
	{
		memset(szUserId,0,30);
		memset(szPower,0,300);
		beUsed=0;
	}
	
}Userpower;
//版本核对
typedef struct  tagSoftVersionInfo
{
   char    szName[20]; //公司的名称
   int     iVersion;   //版本号;
   tagSoftVersionInfo()
   {
	   memset(szName,0,20);
	   iVersion=1;
   }

	
}SoftVersionInfo;
//处理结果
class  COpeateResult
{
public:
	char  USERID[30];
	char  EUQIMENTID[15];
	char  TIME[20];
	char  RESULT[200];
	char  ALARMINFO[50];
	COpeateResult()
	{
		memset(USERID,0,30);
		memset(EUQIMENTID,0,15);
		memset(TIME,0,20);
		COleDateTime  now=COleDateTime::GetCurrentTime();
		sprintf(TIME,"%s",now.Format("%Y-%m-%d %H:%M:%S"));
		memset(RESULT,0,200);
		memset(ALARMINFO,0,50);
	}
	
};
//通知客户
class CMessageToClient
{
public:
	char szTel[15];
	char Message[200];
	CMessageToClient()
	{
		memset(szTel,0,15);
		memset(Message,0,200);
	}
	
};
//任务安排
class CTaskToWorker
{
public:
	char szTel[15];
	char Message[200];
	CTaskToWorker()
	{
		memset(szTel,0,15);
		memset(Message,0,200);
	}
	
};
//控制手机信息
class  CCtrTels
{
public:
	char EUQIMENTID[15];
	char TELS[200];
	CCtrTels()
	{
		memset(EUQIMENTID,0,15);
		memset(TELS,0,200);
	}
};
//报警接受手机信息
class CAlarmRecvTels
{
public:
	char  EQUIMENTID[15];
	char  CONTACTTEL[200];
	CAlarmRecvTels()
	{
		memset(EQUIMENTID,0,15);
		memset(CONTACTTEL,0,200);
	}
};
//费用信息
class  CFeeInfo
{
public:
	char  EQUIMENTID[15];
	char  SERVERTYPE[30];
	char  OPENTIME[20];
	char  FEETIME[20];
	int   FEE;
	char  STRATTIME[20];
    char  ENDTIME[20]; 
	char  CompanyNo[30];
	CFeeInfo()
	{
		memset(EQUIMENTID,0,15);
		memset(SERVERTYPE,0,30);
		memset(OPENTIME,0,20);
		memset(FEETIME,0,20);
		FEE=0;
		memset(STRATTIME,0,20);
		memset(ENDTIME,0,20);
		memset(CompanyNo,0,20);
		
	}
	
};

//用户地图信息
class  CUserMapInfo
{
public:
	char  USERID[30];
	char  MAPNAME[200];
	CUserMapInfo()
	{
		memset(USERID,0,30);
		memset(MAPNAME,0,200 );
	}
	
};

//区域信息
class  CAreaInfo
{
public:
	char  ID[10];
	char  NAME[30];
	int   TYPE;
	int   VALID;
	double   SPEED;
	char  PATH[256];
	char  CREATETIME[20];
	char  INVALIDTIME[20];

	CAreaInfo()
	{
		memset(ID, 0, 10);
		memset(NAME, 0, 30);
		TYPE = 0;
		VALID = 1;
		SPEED = 0;
		memset(PATH, 0, 256);
		memset(CREATETIME, 0, 20);
		memset(INVALIDTIME, 0, 20);
	}
};

class  CEventInfo
{
public:
	char  EQUIPMENTID[15];
	char  PEERID[15];
	int   TYPE;
	char  BEGINTIME[20];
	char  ENDTIME[20];
	char  AREANAME[30];
	double   SPEED;
	double   SpeedLimit;

	class  CEventInfo()
	{
		memset(EQUIPMENTID, 0, 15);
		memset(PEERID, 0, 15);
		TYPE = 1;
		memset(BEGINTIME, 0, 20);
		memset(ENDTIME, 0, 20);
		memset(AREANAME, 0, 30);
		SPEED = 0;
		SpeedLimit = 0;
	}
};

#define WM_GPSDATA_MSG WM_USER + 100

#endif
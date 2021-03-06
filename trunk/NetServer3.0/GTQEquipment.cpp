// GTQEquipment.cpp: implementation of the CGTQEquipment class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DBWork.h"
#include "GTQEquipment.h"
#include "NetServer.h"
#include "fstream"
using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern  CNetServerApp   theApp;
extern  CMutex g_SpeedAlarmLock;
int GTQGPSCount=0;
void      _GTQLog(CString szLog)
{
	static  int icount=0;
	COleDateTime    now=COleDateTime::GetCurrentTime();
	CString  szTemp;
	szTemp.Format("%d   ",GTQGPSCount++);
	szTemp+=szLog;
	CString    szFileName;
	szFileName.Format("Log\\GTQ%s.log",now.Format("%Y%m%d"));
	ofstream   f(szFileName.GetBuffer(0),ios::app|ios::out);
	
//#ifdef  _DEBUG
//	//这个部分负责记录程序在调试的时候的过程;
//	char buf[4096]={0};
//	sprintf(buf,"%s",szTemp);
//    f<<buf<<endl;
//	f.close();
//	return ;
//#endif	
    f<<szTemp.GetBuffer(0)<<endl;
	f.close();
	return ;
	
}
extern CDBWork  g_DBWork;
#define TQMAXCOMMANDID  8
#define   MINLENTCP    19  
#define   MINLENUDP    20
//V1
typedef  struct  tagGTQV1Data 
{
	char  Begin;
	char  MakeName[3];	
	char  szKey[11];	
	char  TYPE1[3];	
	
	char  Time[7];	
	char  Able[2];	
	char  Lat[10];	
	char  LatPoint[2];	
	char  Longtinue[11];	
	char  LONGPoint[2];
	char  Speed[7];
	char  Diredtion[4];
	char  Date[7];
	char  state[8];
	char  End;
	
	void  init()
	{
		MakeName[2]=0;
		szKey[10]=0;
		TYPE1[2]=0;
		
		Time[6]=0;
		Able[1]=0;
		Lat[9]=0;
		LatPoint[1]=0;
		Longtinue[10]=0;
		LONGPoint[1]=0;
		Speed[6]=0;
		Diredtion[3]=0;
		Date[6]=0;


	}
	//后，头的数据有变化的部分，用变体来处理
}GTQV1Data;


//V4  2
typedef  struct  tagGTQV42Data 
{
	char  Begin;
	char  MakeName[3];	
	char  szKey[11];	
	char  TYPE1[3];	
	char  TYPE2[3];
	char  settime[7];
	char  Time[7];	
	char  Able[2];	
	char  Lat[10];	
	char  LatPoint[2];	
	char  Longtinue[11];	
	char  LONGPoint[2];
	char  Speed[7];
	char  Diredtion[4];
	char  Date[7];
	char  state[8];
	char  End;
	void  init()
	{
		MakeName[2]=0;
		szKey[10]=0;
		TYPE1[2]=0;
		TYPE2[2]=0;
		settime[6]=0;
		Time[6]=0;
		Able[1]=0;
		Lat[9]=0;
		LatPoint[1]=0;
		Longtinue[10]=0;
		LONGPoint[1]=0;
		Speed[6]=0;
		Diredtion[3]=0;
		Date[6]=0;
		
	}
	//后，头的数据有变化的部分，用变体来处理
}GTQV42Data;

//V4  3
typedef  struct  tagGTQV43Data 
{
	char  Begin;
	char  MakeName[3];	
	char  szKey[11];	
	char  TYPE1[3];	
	char  TYPE2[4];
	char  Time[7];	
	char  Able[2];	
	char  Lat[10];	
	char  LatPoint[2];	
	char  Longtinue[11];	
	char  LONGPoint[2];
	char  Speed[7];
	char  Diredtion[4];
	char  Date[7];
	char  state[8];
	char  End;
	void  init()
	{
		MakeName[2]=0;
		szKey[10]=0;
		TYPE1[2]=0;
		TYPE2[3]=0;
		Time[6]=0;
		Able[1]=0;
		Lat[9]=0;
		LatPoint[1]=0;
		Longtinue[10]=0;
		LONGPoint[1]=0;
		Speed[6]=0;
		Diredtion[3]=0;
		Date[6]=0;
		
	}
	//后，头的数据有变化的部分，用变体来处理
}GTQV43Data;

//压缩后的数据;
typedef  struct  tagGTQPREDATA
{
	char  Begin;
	char  MakeName[3];	
	char  szKey[11];	
	char  TYPE1[3];	
	char  TYPE2[4];
	char  Time[7];	
	char  Able[2];	
	char  Lat[10];	
	char  LatPoint[2];	
	char  Longtinue[11];	
	char  LONGPoint[2];
	char  Speed[7];
	char  Diredtion[4];
	char  Date[7];
	char  state[8];
	char  End;
	void  init()
	{
		MakeName[2]=0;
		szKey[10]=0;
		TYPE1[2]=0;
		TYPE2[3]=0;
		Time[6]=0;
		Able[1]=0;
		Lat[9]=0;
		LatPoint[1]=0;
		Longtinue[10]=0;
		LONGPoint[1]=0;
		Speed[6]=0;
		Diredtion[3]=0;
		Date[6]=0;
		
	}
	//后，头的数据有变化的部分，用变体来处理
}GTQPREDATA;

//短信息的头文件
//*XX,YYYYYYYYYY,I1,HHMMSS,1,Code,Info_lenth,Information#
typedef  struct  tagGTQMESSAGE
{
	char  Begin;
	char  MakeName[3];	
	char  szKey[11];	
	char  TYPE[3];		
	char  Time[7];	
	char  Fix[2];	  //1
	char  Code[2];	
	void  init()
	{
		MakeName[2]=0;
		szKey[10]=0;
		TYPE[2]=0;	
		Time[6]=0;
		Fix[1]=0;
		Code[1]=0;		
	}
}GTQMESSAGE;

class   GBCDCHAR
{
public:
	unsigned short    B1 :4;
	unsigned  short    B2 :4;
};
CString   szGReType2="I1,";  //需要确认的晖映信息
CString   szGReType3="";  //需要确认的晖映信息



static bool UnicodeNeeded(/*[in]*/ BYTE* pIn, /*[out]*/ BYTE* pOut, /*[in]*/ int unicodeLen/*[out]*/,int* charlen)
{
	//	test if unicode is needed
	bool u = false;
	::ZeroMemory(pOut, 512);	
	for (int i = 0; i < unicodeLen; i++)
	{
		BYTE t = pIn[i];
		pIn[i] = pIn[i + 1];
		pIn[i++ + 1] = t;
	}
	BOOL UsedDefaultChar = FALSE;  
	*charlen = ::WideCharToMultiByte(CP_THREAD_ACP, WC_COMPOSITECHECK, (wchar_t*)pIn, unicodeLen/2, (char*)pOut, 512, NULL, &UsedDefaultChar);
	return u;
}

void   __GTQSendGpsLog(char  * buf)
{
	ofstream  f("GTQSendGpsLog.Log",ios::app| ios::out);
	f<<buf<<endl;
	f.close();
}



void   __ErrorLog(CString &log)
{
	char buf[1024]={0};
	sprintf(buf,"%s",log);
	ofstream  f("ErrorLog.Log",ios::app| ios::out);
	f<<buf<<endl;
	f.close();
}


CGTQEquipment::CGTQEquipment()
{

}

CGTQEquipment::~CGTQEquipment()
{

}
BOOL   CGTQEquipment::IsDataLegalAndFindPackNum(char *p,int  iLen,int& INumber,int &iEnd)
{
	if(!p)
		return FALSE;
	INumber=0;
	if(iLen<MINLENTCP || iLen<MINLENUDP)
		return  FALSE;
  

	for(int i=1;i<iLen;i++)
	{
		if(p[i]=='#')
		{
			INumber++;
			iEnd=i;
		}
				
	}

	if(p[0]=='$')
	{
	}
	return TRUE;
}
BOOL   CGTQEquipment::IsRegiste(CIOCPBuff  *pIOBuf)
{
	//*HQ,3070320141,V1,075744,A,3116.8474,N,12127.8375,E,000.00,344,300407,FBFFFBFF#
	if(!pIOBuf  || !pIOBuf->buf  || pIOBuf->buf[0]!='*')
		return FALSE;
	char  *pData=pIOBuf->buf;
//	memcpy(pIOBuf->szKey,&pData[4],10);
//	pIOBuf->szKey[10]=0;

	if(pData[15]=='V' && pData[16]=='1')
		return  TRUE;
	if(pData[15]=='V' && pData[16]=='4')
		return  TRUE;

//	if(pData[15]=='I' && pData[16]=='1')
//		return  TRUE;
	return  TRUE;

}
BOOL CGTQEquipment::ParseGpsData(CIOCPBuff *pIOBuf,CGPSData  &GpsData,CString &szMessage)
{
	if(!pIOBuf)
		return FALSE;
	int iNumber=0,iEnd=pIOBuf->iMsgLen;
    if(!IsDataLegalAndFindPackNum(pIOBuf->buf,pIOBuf->iMsgLen,iNumber,iEnd))
	{	
		return  FALSE;
	}
	if(IsRegiste(pIOBuf))
	{//进行设备的注册；主要是记录设备的连接信息；
        
		CString   szKey;		 
		char  *p=pIOBuf->GetWSABuffer()->buf;
		char   temp[15]={0};
		memcpy(temp,&p[4],10);
		szKey=temp;
		this->m_EquimenList.UpdatUser(pIOBuf->fd,szKey,pIOBuf->addr);		
	}
	BOOL  b=FALSE;
	b=DecGpsData(pIOBuf,GpsData,szMessage);
	if(!b)
	{
		CString  szStr;
		if(GetDCBTOASC(pIOBuf->buf, pIOBuf->iMsgLen, szStr))
		{
			__ErrorLog(szStr);
		}
		else
		{
			CString  szError="error";
			szError=pIOBuf->buf;
			__ErrorLog(szStr);
			
		}
	}
	return b;
//	return  DecGpsData(pIOBuf,GpsData,szMessage);

}

BOOL CGTQEquipment::DecGpsData(CIOCPBuff *p, CGPSData &GpsData,CString &szMessage)
{
	if(!p)
		return  FALSE;
   char *pData=p->buf;
   if(!pData)
	   return FALSE;
   GTQV1Data *pTQV1Data=NULL;
   GTQV42Data *pTQV42Data=NULL;
   GTQV43Data *pTQV43Data=NULL;
   if(pData[0]=='*')
   {//未压缩的数据;
	   CString   szNoyeTime=pData;
       int   SubTypeLen=0;
	   GTQDATATYPE iType=QueryDataType(pData,p->iMsgLen,SubTypeLen);
	   switch(iType) 
	   {
	   case  GTYPE_V1:
		   {//一般的数据信息;
			   pTQV1Data=(GTQV1Data * )(pData=p->buf);
			   pTQV1Data->init();
			   GpsData.fLongitude=DecLONG(pTQV1Data->Longtinue);

			   GpsData.fLatitude=DecLat(pTQV1Data->Lat);
			   GpsData.fSpeed=(float)atof(pTQV1Data->Speed);
			   GpsData.iDirection=atoi(pTQV1Data->Diredtion);
			   GpsData.iState=SetState(pTQV1Data->state);
			   if(pTQV1Data->Able[0]=='A')
				   GpsData.iFlag=6;
			   else
			   {
				   GpsData.iFlag=0;
			//	   return  FALSE;
			   }
				          
			   //侧测试的时候用的
			//   COleDateTime  now=COleDateTime::GetCurrentTime();
			   COleDateTime  now=GetNOYSTime(szNoyeTime);
			   CString     szTime=now.Format("%Y-%m-%d %H:%M:%S");
			   sprintf(GpsData.Time,"%s",szTime);
		//	   GpsData.Time[20]=0;
			   //end;
			   memcpy(GpsData.szEquimentID,pTQV1Data->szKey,10);  //得到设备的key;
			   char  szSta[9]={0};
			   memcpy(szSta,pTQV1Data->state,8);
			   SetSata(szSta,GpsData);
			   if(GpsData.iState==GPS_ROBALARM)
				   return TRUE;
			   //超速度
			   if(GpsData.fSpeed>=80)
				   return FALSE;
			   g_SpeedAlarmLock.Lock();
			   int iCount=theApp.m_asSpeedAlarmInfo.GetSize();
			   for(int iLoop=0;iLoop<iCount;iLoop++)
			   {
				   CSpeedAlarmInfo  info=theApp.m_asSpeedAlarmInfo.GetAt(iLoop);
				   if(info.szKeyId==GpsData.szEquimentID)
				   {
					   if(GpsData.fSpeed>=info.iHSpeed)
					   {
						   GpsData.iState=GPS_HIGHSPEEDALARM;
						   CGpsCommand  ppC;
						   CString  sasasa="11111111111";
						   // ppC.EquimentId;
						   sprintf(ppC.EquimentId,"%s",sasasa);
						   ppC.iCOmmandID=4;
						   CString saasa="你已超速";
						   sprintf(ppC.Param,"%s",saasa);
						   this->SendTQGPSCommand(info.szKeyId,&ppC);
						   
					   }
					   if(GpsData.fSpeed<info.iHSpeed  && (GpsData.fSpeed>=(info.iHSpeed-2)))
					   {
						   CGpsCommand  ppC;
						   CString  sasasa="11111111111";
						   // ppC.EquimentId;
						   sprintf(ppC.EquimentId,"%s",sasasa);
						   ppC.iCOmmandID=4;
						   CString saasa="请减速";
						   sprintf(ppC.Param,"%s",saasa);
						   this->SendTQGPSCommand(info.szKeyId,&ppC);
					   }
					   break;
				   }
			   }
			   
			   g_SpeedAlarmLock.Unlock();
	
		   }
	   	break;
	   case GTYPE_V4:
		   {
			   if(SubTypeLen==2)
			   {
				   
				   pTQV42Data=(GTQV42Data * )(pData=p->buf);
				   pTQV42Data->init();
				   if(szGReType2.Find(pTQV42Data->TYPE2)==-1)
					   return  FALSE;  //不是一开始定义的就不用理会;
				   GpsData.fLongitude=DecLONG(pTQV42Data->Longtinue);
				   
				   GpsData.fLatitude=DecLat(pTQV42Data->Lat);
				   GpsData.fSpeed=(float)atof(pTQV42Data->Speed);
				   GpsData.iDirection=atoi(pTQV42Data->Diredtion);
				   GpsData.iState=SetState(pTQV42Data->state);
				   if(pTQV42Data->Able[0]=='A')
					   GpsData.iFlag=6;
				   else
				   {
					  
					   GpsData.iFlag=0; 
				//	    return  FALSE;
				   }
				   //侧测试的时候用的
			//	   COleDateTime  now=COleDateTime::GetCurrentTime();
				   COleDateTime  now=GetNOYSTime(szNoyeTime);
				   CString     szTime=now.Format("%Y-%m-%d %H:%M:%S");
				   sprintf(GpsData.Time,"%s",szTime);
				   memcpy(GpsData.szEquimentID,pTQV42Data->szKey,10);  //得到设备的key;
                   //判断回应信息的内容;
				   CString  szReMessage=pTQV42Data->TYPE2;
				   if(szReMessage.Find("I1")!=-1)
				   {//短消息正确接受;
					   GpsData.iState=GPS_SENDMESSAGEOK;
				   }			   
			   }
			   if(SubTypeLen==3)
			   {
				   pTQV43Data=(GTQV43Data * )(pData=p->buf);
				   pTQV43Data->init();
			   }
			  // 设备回应的信息
	
		   }
	   	break;
	   case  GTYPE_I1:
		   {
			   //短消息
			   char  buf109[7]={0};
			   memcpy(buf109,pData+18,6);
			   CString  szReCommandID=buf109;

			   DOTQMessage(pData,p->iMsgLen,szMessage);
			   memcpy(GpsData.szEquimentID,pData+4,10);  //得到设备的key;

			   GpsData.iFlag=0;
			   COleDateTime  now=COleDateTime::GetCurrentTime();
			   CString     szTime=now.Format("%Y-%m-%d %H:%M:%S");
			   sprintf(GpsData.Time,"%s",szTime);
			   szMessage=szMessage+"#";
			   szMessage=szMessage+szReCommandID;
		   }
		   break;
		    
	   case  GTYPE_I8:
		   { //从计价器发过来的信息;

			   CString  szStr;
			   if(GetDCBTOASC(pData,p->iMsgLen,szStr)==FALSE)
				   return  FALSE;
			  memcpy(GpsData.szEquimentID,pData+4,10);  //得到设备的key;
              szMessage=GetTaxiData(GpsData.szEquimentID,szStr);
			  GpsData.iFlag=0;
			  COleDateTime  now=COleDateTime::GetCurrentTime();
			  CString     szTime=now.Format("%Y-%m-%d %H:%M:%S");
			  sprintf(GpsData.Time,"%s",szTime);		
		   }
		   break;
	   case  GTYPE_I2:
		   {
			   memcpy(GpsData.szEquimentID,pData+4,10);  //得到设备的key;
			   COleDateTime  now=COleDateTime::GetCurrentTime();
			   CString     szTime=now.Format("%Y-%m-%d %H:%M:%S");
			   sprintf(GpsData.Time,"%s",szTime);
			   GpsData.iFlag=0;
			   char   chMessage[5]={0};

			   if(szNoyeTime.Find(",A")!=-1)
			   {
				   int index=szNoyeTime.Find(",A");
				   memcpy(chMessage,&(pData[index+2]),4);
				   szMessage.Format("驾驶员%s登陆",chMessage);
			   }
			   if(szNoyeTime.Find(",V")!=-1)
			   {
				    int index=szNoyeTime.Find(",A");
				   szMessage.Format("驾驶员离线");
			   }
		   }
		   break;
	   default:
		   ASSERT(FALSE);
		   return FALSE;
	   }	   
	   

	   return  TRUE;
   }
   //$3070320141084211300407  3116853500 121278339E000168FBFFFBFFFF0048
   if(pData[0]=='$')
   {//压缩的数据;
      GTQPREDATA  *pPreData=NULL;
	   pPreData=(GTQPREDATA * )(pData);
	   if(pPreData==NULL)
		   return FALSE;
	   CString  szStr;
	   if(GetDCBTOASC(pData,p->iMsgLen,szStr)==FALSE)
		   return  FALSE;
	   int  iLen=szStr.GetLength();
	   if(iLen<60)
		   return FALSE;
	   CString szId=szStr.Mid(1,10);
	   CString  szLong,szLat;
	   szLat=szStr.Mid(23,8);
	   szLong=szStr.Mid(33,9);
	   szLat.Insert(4,".");
	   szLong.Insert(5,".");
	   GpsData.fLongitude=DecLONG(szLong.GetBuffer(0));
	   
	   GpsData.fLatitude=DecLat(szLat.GetBuffer(0));
	   GpsData.fSpeed=(float)0;
	   GpsData.iDirection=0;
	   GpsData.iState=SetState(pTQV1Data->state);
	   int Eindex=szStr.Find("E",33);
	   if(Eindex!=-1)
	   {
		    GpsData.iFlag=6;
			//速度和方向;
			CString  szSpeed=szStr.Mid(Eindex+1,3);
			CString  szOrinet=szStr.Mid(Eindex+4,3);
			GpsData.fSpeed=atoi(szSpeed.GetBuffer(0)) *1.852;
            GpsData.iDirection=atoi(szOrinet.GetBuffer(0));

	   }

	   else
	   {
		   GpsData.iFlag=0; 
	//	    return  FALSE;
	   }
		   

	   
	   //侧测试的时候用的
	 //  COleDateTime  now=COleDateTime::GetCurrentTime();
	    COleDateTime  now=GetYSTime(szStr);
	   CString     szTime=now.Format("%Y-%m-%d %H:%M:%S");
	   sprintf(GpsData.Time,"%s",szTime);   
	   memcpy(GpsData.szEquimentID,szId.GetBuffer(0),10);
//	   if(this->GetTaxibeWork(GpsData.szEquimentID)==TRUE)
	   {
//		   GpsData.iState=GPS_STATONE;
	   }
	   //设置状态
	   CString   szSta=szStr.Mid(iLen-6-8,8);
	  // SetSata(szSta,GpsData);
	   SetUDPSata(szSta,GpsData);
	   if(GpsData.iState==GPS_ROBALARM)
		   return TRUE;
	   if(GpsData.fSpeed>=80)
		   return FALSE;
	   //超速度
	   g_SpeedAlarmLock.Lock();
	   int iCount=theApp.m_asSpeedAlarmInfo.GetSize();
	   for(int iLoop=0;iLoop<iCount;iLoop++)
	   {
		   CSpeedAlarmInfo  info=theApp.m_asSpeedAlarmInfo.GetAt(iLoop);
		   if(info.szKeyId==GpsData.szEquimentID)
		   {
			   if(GpsData.fSpeed>=info.iHSpeed)
			   {
				   GpsData.iState=GPS_HIGHSPEEDALARM;
				   //				   m_tempszMessage+="超速度";
				   CGpsCommand  ppC;
				   CString  sasasa="11111111111";
				   // ppC.EquimentId;
				   sprintf(ppC.EquimentId,"%s",sasasa);
				   ppC.iCOmmandID=4;
				   CString saasa="你已经超速";
				   sprintf(ppC.Param,"%s",saasa);
				   this->SendTQGPSCommand(info.szKeyId,&ppC);
				   
			   }
			   if(GpsData.fSpeed<info.iHSpeed  && (GpsData.fSpeed>=(info.iHSpeed-2)))
			   {
				   CGpsCommand  ppC;
				   CString  sasasa="11111111111";
				   // ppC.EquimentId;
				   sprintf(ppC.EquimentId,"%s",sasasa);
				   ppC.iCOmmandID=4;
				   CString saasa="请件速";
				   sprintf(ppC.Param,"%s",saasa);
				   this->SendTQGPSCommand(info.szKeyId,&ppC);
			   }
			   break;
		   }
	   }
	   
	   g_SpeedAlarmLock.Unlock();
   }
   else
   {
	   
	   _GTQLog("不合法的数据");
	   return  FALSE;
   }

	return TRUE;
}

//判断数据的类型;
GTQDATATYPE CGTQEquipment::QueryDataType(char  *p,int ilen,int &SubTypeLen)
{
	if(!p)
		return GTYPE_TQNULL; 
	if(p[0]=='$')
		return  GTYPE_PRESS;   //压缩数据的标志;
	SubTypeLen=0;
	if(p[15]=='V'  && p[16]=='1')
		return  GTYPE_V1;
	if(p[15]=='I'  && p[16]=='1')
		return  GTYPE_I1;
	if(p[15]=='I'  && p[16]=='2')
		return  GTYPE_I2;
	if(p[15]=='I'  && p[16]=='8')
		return  GTYPE_I8;
	if(p[15]=='V'  && p[16]=='4')
	{
		for(int i=18;i<ilen;i++)
		{
			if(p[i]==',')
			{
                SubTypeLen=i-18;
				break;
			}
		}
		return  GTYPE_V4;

	}
	return GTYPE_TQNULL;

}
UINT CGTQEquipment::SetState(char *pState)
{
	return  0;

}
//BCD串转化成ASc串
BOOL CGTQEquipment::GetDCBTOASC(char *buf, int iLen, CString &szStr)
{
	if(buf==NULL)
	{
		_GTQLog("buf==NULL");
		return  FALSE;
	}
	szStr.Empty();
	szStr.Format("%c",buf[0]);
	for(int  iLoop=1;iLoop<iLen;iLoop++)
	{
	//	char  c=buf[iLoop];
		GBCDCHAR  *pBcd=(GBCDCHAR *)(&buf[iLoop]);
		CString  szTemp;
		char  b1,b2;
		if((pBcd->B2)>9)
		{
	       b2=pBcd->B2+55;

		}
		else
		{
			b2=pBcd->B2+0x30;
		}

		if((pBcd->B1)>9)
		{
			b1=pBcd->B1+55;
			
		}
		else
		{
			b1=pBcd->B1+0x30;
		}
	//	b1=pBcd->B1;
	//	b2=pBcd->B2;
		szTemp.Format("%c%c",b2,b1);
		szStr+=szTemp;
	}
	_GTQLog(szStr);
	return  TRUE;

}

//处理所有的命令的函数;
BOOL CGTQEquipment::SendTQGPSCommand(CString szEquimentKey, const CGpsCommand *pCommand)
{
	CString  szLog;
	szLog.Format("CMTGPSEquipment::SendTQGPSCommand begin  %s  ",szEquimentKey);
	char  logbuf[1000]={0}; 
	sprintf(logbuf,"%s",szLog);
	__GTQSendGpsLog(logbuf);
	
	if(!pCommand)
		return FALSE;
	EquiNetInfo *pNetInfo=this->m_EquimenList.Find(szEquimentKey);
	//发送命令
	if(!pNetInfo)
	{
		szLog="CGTQEquipment::SendMTGPSCommand pNetInfo  ==NULL  return  FALSE ";
		memset( logbuf,0,1000);
		sprintf(logbuf,"%s",szLog);
		__GTQSendGpsLog(logbuf);
		return FALSE;
	}
	
	struct  sockaddr_in   addr=pNetInfo->addr;
	// addr.sin_port=htons(addr.sin_port);
	int   fd=pNetInfo->fd;
	if(fd==INVALID_SOCKET)
	{
		szLog="fd==INVALID_SOCKET  return  FALSE ";
		memset( logbuf,0,1000);
		sprintf(logbuf,"%s",szLog);
		__GTQSendGpsLog(logbuf);
		return  FALSE;
	}
	char bufcmd[1024];
	if(!PackCommand(pCommand,bufcmd,1024))
	{
		szLog="CGTQEquipment::SendTQGPSCommand 不能解析控制命令  return  FALSE ";
		memset( logbuf,0,1000);
		sprintf(logbuf,"%s",szLog);
		__GTQSendGpsLog(logbuf);
		return  FALSE;
	}
	//  CString  szTemp="*13370270190,000000,S01010111#ff";
	//  sprintf(bufcmd,"%s",szTemp);
	// int i=m_SendSock.SendTo(bufcmd,strlen(bufcmd),(SOCKADDR *)&addr,sizeof(pNetInfo->addr),0);
	
	// Sendfd
//	int i=sendto(fd,bufcmd,strlen(bufcmd),0,(SOCKADDR *)&addr,sizeof(pNetInfo->addr));
    int i=send(fd,bufcmd,1024,0);
	
	if(i<=0)
	{
		szLog="i<=0  return  FALSE ";
		memset( logbuf,0,1000);
		sprintf(logbuf,"%s",szLog);
		__GTQSendGpsLog(logbuf);
		
		return FALSE;
	}
	int irroe=WSAGetLastError();
	//en
	__GTQSendGpsLog(bufcmd);
	return  TRUE;
}
BOOL CGTQEquipment::PackCommand(const CGpsCommand *pCommand,char buf[],int iLen)
{
	CString  szCommandinfo[8]={"锁车","解除锁车","报警确认","解除报警","短信息","上报时间设置",
		"超速度设置","监听设置"};

	if(!pCommand)
		return FALSE;
	CString  szEquimentKey=pCommand->EquimentId;

	if(szEquimentKey.IsEmpty())
		return  FALSE;
	int  iCommandID=pCommand->iCOmmandID;
	if(iCommandID==USER_DEFINED)
	{//用户自定义的命令
		CString  szText=pCommand->Param;
		szText.TrimLeft(),szText.TrimRight();
		if(szText.IsEmpty())
			return FALSE;
		memset(buf,0,iLen);
		sprintf(buf,"%s",szText);
		return  TRUE;
	}
	if(iCommandID<0  ||iCommandID>=TQMAXCOMMANDID)
	{
		return  FALSE;
	}
	//*FFFFFFFFFFF,PPPPPP,XXXX,ZZ…ZZ#CC
	
	CString szCommandText=szCommandinfo[iCommandID];
	if(szCommandText.IsEmpty())
		return  FALSE;
	CString  szCommand;  
	if(szCommandText.Find("超速度设置")!=-1)
	{//chapsudu shezhi 
		CEquimentInfo  info;
		g_DBWork.GetEquimentInfo(szEquimentKey,info);

		CString  szsendpar=pCommand->Param;
		int  i=szsendpar.Find(",");
		if(i==-1)
		{
			return  FALSE;
		}
		CString  szLs=szsendpar.Mid(0,i);
		CString  szHs=szsendpar.Mid(i+1);
		float  fspeed=atof(szHs.GetBuffer(0));
		float ffspeed=fspeed;
		int  iSpeed2=(int)ffspeed;
		g_DBWork.DelSpeedAlarm(info.EquimentKey);
		fspeed=atof(szLs.GetBuffer(0));
		int  iSpeed1=(int )fspeed;
		if(g_DBWork.SetSpeedAlarm(info.EquimentKey,iSpeed1,iSpeed2)>0)
		{
		}
		else
		{
			return FALSE;
		}
	}
	if(szCommandText.Find("上报时间设置")!=-1)
	{
		//*XX,YYYYYYYYYY,D1,HHMMSS,interval,batch#
		CString  szPWS=pCommand->Param;
		int  itv=atoi(szPWS.GetBuffer(0));
		if(itv<=0 ||itv>=255)
			return  FALSE;
		szCommand.Format("*HQ,000,D1,121212,%d,1#",itv);
		g_DBWork.UpdateEquimentTimeSet(szEquimentKey,szCommand);
		//纪录下他的发送时间;
		
	//	szCommand.Format("*HQ,000,S2,121212,111#");


	}
	if(szCommandText.Find("短信息")!=-1)
	{
		//*XX,YYYYYYYYYY,I1,HHMMSS,Display_Time,Code,Info_lenth,Information
        CString  szPAR=pCommand->Param;
		int  iLen;
		CString  szpws2=pCommand->PWS;
        return PackSMSCommand(buf,iLen,szpws2,szPAR);
			
	}
	//锁车
	if(szCommandText.Find("锁车")!=-1)
	{
		//*XX,YYYYYYYYYY,I1,HHMMSS,Display_Time,Code,Info_lenth,Information
		szCommand.Format("*HQ,000,S20,130305,1,3,10,3,5,5,3,5,3,5,3,5#");		
	}
	//
	if(szCommandText.Find("解除锁车")!=-1)
	{
		szCommand.Format("*HQ,000,S20,130305,1,0#");		
	}
	//监听设置
	if(szCommandText.Find("监听设置")!=-1)
	{
		//*XX,YYYYYYYYYY,R8,HHMMSS,listen_address #
		CString  szTel=pCommand->Param;
		szCommand.Format("*HQ,000,R8,130305,%s#",szTel);		
	}
	memset(buf,0,iLen);
	sprintf(buf,"%s",szCommand);
	return  TRUE;	

}


//解析短信息；*XX,YYYYYYYYYY,I1,HHMMSS,1,Code,Info_lenth,Information#
BOOL CGTQEquipment::DOTQMessage(char *pBuf, int iLen,CString  &szMessage)
{
    if(!pBuf)
		return  FALSE;
	if(iLen<sizeof(GTQMESSAGE))
		return  FALSE;

	CString  szStr=pBuf;
	if(szStr.GetLength() <  sizeof(GTQMESSAGE))
		return  FALSE;
	
	int  index=szStr.Find(",",sizeof(GTQMESSAGE));
	if(index==-1)
		return  FALSE;
	CString  szino_Len=szStr.Mid(sizeof(GTQMESSAGE),index-sizeof(GTQMESSAGE));
	int  iiLen=atoi(szino_Len.GetBuffer(0));
	if(iiLen<=0)
		return  FALSE;

	//扶植短信息；
	char  tempbuf[100]={0};
	memcpy(tempbuf,&pBuf[index+1],iiLen);
	CString szeee=tempbuf;
	if(szeee.Find("PIC")!=-1)
	{
		szMessage=szeee;
		return  TRUE;
	}
	if(tempbuf[0]=='1'  &&  tempbuf[1]=='8')
	{//ok按下
		szMessage="抢答";
		return  TRUE;
	}
	BYTE  MTDPackage[512]={0};
//	int len=8;	
	int rilen=-1;
	::UnicodeNeeded((BYTE *)tempbuf,MTDPackage,iiLen,&rilen);
	BYTE  sztempmessage[512]={0};
	memcpy(sztempmessage,MTDPackage,512);
	szMessage=sztempmessage;
	return  TRUE;
	
}


BOOL CGTQEquipment::PackSMSCommand(char *pBuf, int &iLen,CString szPws2, CString &szMessage)
{

	CString szMsg;
	int istrlen=szMessage.GetLength();
	char bufmessage[1024]={0};
	sprintf(bufmessage,"%s",szMessage);
	//加入数字的处理
	char  *buftemp=NULL;
	buftemp=new char[1024];
	
	int icount=0;
	if(buftemp==NULL)
		return  -1;
	memset(buftemp,0,1024);
	//判断非汉字的字
    for(int iLoop=0;iLoop<(szMessage.GetLength());iLoop++)
	{
		char   testbuf[5]={0};
		CString sztempMsg;
		sztempMsg=szMessage.Mid(iLoop,1);
		sprintf(testbuf,"%s",sztempMsg);
		if(testbuf[0] & 0x80)
		{
			
			sztempMsg=szMessage.Mid(iLoop,2);
			iLoop++;
		}
		if(sztempMsg.GetLength()==1)
		{
			//非汉字
			memset(testbuf,0,5);
			sprintf(testbuf+1,"%s",sztempMsg);
		}
		else
		{
			//汉字
			sprintf(testbuf,"%s",sztempMsg);
		}
		//写入内存;
		memcpy(buftemp+icount,testbuf,2);
		icount+=2;
	}  
	if(icount>256)
	{
		icount=256;
	}
	
	szMsg.Format("*HQ,000,I1,%s,60000,0,%d,",szPws2.Mid(0,6),icount);
//	char  buf[1024]={0};
	sprintf(pBuf,"%s",szMsg);
	int iBegin=strlen(pBuf);
	memcpy(pBuf+iBegin,buftemp,icount);
	char endsign[2]="#";
	memcpy(pBuf+iBegin+icount,endsign,1);
	iLen=iBegin+icount+1;
	return  TRUE;

}

CString CGTQEquipment::GetTaxiData(CString  szKey,CString szStr)
{
	CString  szLog=szStr;
	CString  szTemp;
	char   TaxiBuf[1024]={0};
	sprintf(TaxiBuf,"%s",szStr);

	
	int Bindex=szLog.Find("BF02");
	if(Bindex==-1)
	{
		szTemp="?";
		return szTemp;
	}
	int  endexstart=Bindex+4;
	int Eindex=szLog.Find("BF03",endexstart);
	if(Bindex==-1  ||Eindex==-1 )
	{
		szTemp="?";
		return   szTemp;   //不		  
	}
	// 	BF 02 30 30 BF 03   重车;
	//  BF 02 34 34 BF 03   空车
	//BF 02 36 20 02 12 07 15 41 12 07 15 43 02 70 00 00 36 00 00 10 00 00 00 00 20 00 00 19 00 20 47 1C BF 03 数据
//    this->SendAtack(szKey);
	szLog=szLog.Mid(Bindex,Eindex-Bindex+4);
	if(szLog.GetLength()==12)
	{
		//空车  and  重车;
		if(szLog.Find("3030",4)!=-1)
		{//重车
			szTemp="重车";
			
			return  szTemp;
		}
		if(szLog.Find("3434",4)!=-1)
		{//
			szTemp="空车";
			
			return  szTemp;
		}
	}
	else
	{//解析数据;
		//BF0236  20 02 12 07 15 41 12 07 15 43 02 70 00 00 36 00 00 10 00 00 00 00 20 00 00 19 00 20 47 1C BF 03 数据
		if(szLog.Find("36")!=4)
		{
			szTemp="?";
			return  szTemp;   //edited  20061219;  //修正测试;
		}
		CString szUpTime=szLog.Mid(6,12);//上车时间
		CString  szDownTime=szLog.Mid(18,8);//下车时间;
		CString  szprice=szLog.Mid(26,4);//单价格;
		CString  szDidtance=szLog.Mid(30,6);//里程
		CString  szwaiteTime=szLog.Mid(36,6);  //等候时间
        CString  szleisureTime=szLog.Mid(42,6);//空闲时间
		CString  szsum=szLog.Mid(48,6);//金额;
		//对这些数据进行处理;
		CString  szTemp;
		szTemp.Format("%s-%s-%s %s:%s:%s",szUpTime.Mid(0,2),szUpTime.Mid(2,2),szUpTime.Mid(4,2),szUpTime.Mid(6,2),szUpTime.Mid(8,2),szUpTime.Mid(10,2));		
		CString   szSCSJ=szTemp;//上车时间
		CString   szXCSJ;
		szTemp.Empty();
		szTemp.Format("%s-%s %s:%s",szDownTime.Mid(0,2),szDownTime.Mid(2,2),szDownTime.Mid(4,2),szDownTime.Mid(6,2));
		szXCSJ=szTemp;//下车时间;
		int i=atoi(szprice.GetBuffer(0));
		float  iPrice=((float)i)/100;//单价格;
		float  fDistace=(atof(szDidtance.GetBuffer(0)))/100;//里程
		szTemp.Empty();
		szTemp.Format("%s:%s:%s",szwaiteTime.Mid(0,2),szwaiteTime.Mid(2,2),szwaiteTime.Mid(4,2));
		szwaiteTime=szTemp;//等候时间
		szTemp.Empty();
		szTemp.Format("%s:%s:%s",szleisureTime.Mid(0,2),szleisureTime.Mid(2,2),szleisureTime.Mid(4,2));
		szleisureTime=szTemp;//空闲时间
		float   fqian=(atof(szsum.GetBuffer(0)))/100;//金额;
		szTemp.Empty();
		szTemp.Format("交易信息:上车时间:%s  下车时间:%s 单价%.2f元 里程%.1fkm 等候时间%s 空闲时间%s  金额%.2f元 ",szSCSJ,szXCSJ,iPrice,fDistace,szwaiteTime,szleisureTime,fqian);
		return  szTemp;
	}
}


void CGTQEquipment::SetTaxistate(CString szKey,BOOL  beWork)
{

	szKey.TrimLeft(),szKey.TrimRight();
	if(szKey.IsEmpty())
		return;
	int  iCount=m_TaxiWorkArray.GetSize();

	if(beWork==TRUE)
	{//装客阶段
		for(int  iLoop=0;iLoop<iCount;iLoop++)
		{
			CString  temp=m_TaxiWorkArray.GetAt(iLoop);
			if(szKey==temp)
				return;
		}
		m_TaxiWorkArray.Add(szKey);
	}
	if(beWork==FALSE)
	{
		for(int  index=0;index<iCount;index++)
		{
			CString  temp=m_TaxiWorkArray.GetAt(index);
			if(szKey==temp)
			{
				m_TaxiWorkArray.RemoveAt(index);
				return;
			}
				
		}
		return;

	}
	return ;

     
}

BOOL CGTQEquipment::GetTaxibeWork(CString szKey)
{
	szKey.TrimLeft(),szKey.TrimRight();
	if(szKey.IsEmpty())
		return  FALSE;
	int iCount=m_TaxiWorkArray.GetSize();
	for(int iLoop=0;iLoop<iCount;iLoop++)
	{
		CString  szTmep=m_TaxiWorkArray.GetAt(iLoop);
		if(szKey==szTmep)
			return  TRUE;
	}
	return  FALSE;

}

//用来设置设备的状态的函数;
void CGTQEquipment::SetSata(CString szState, CGPSData &gpsData)
{
	gpsData.iState=0;
	if(szState.GetLength()<8)
		return;
	byte  b=0;	
	b=szState.GetAt(7);//报警位;
	b=Getmembyte(b);
	if((b & 0x2)==0)
	{
		// 
		gpsData.iState=GPS_ROBALARM;
		return;
	}
	
	//	b=szState.GetAt(4);
	if((b & 0x4)==0)
		//	if((b &  0x08)==0)
	{
		gpsData.iState=GPS_HIGHSPEEDALARM;
		return;
		//发送一个报警解除
	//	CString  szText="*HQ,000,R7,121212#";
	//	SendTQGPSCommandStr(gpsData.szEquimentID,szText);
		//end
	}
	
	b=szState.GetAt(7);
	if(  (b &  0x8 )==0)
	{
		//		LocationInfo.ulStatus |= 0x00000002;  //非法点火;
		
		//		szMsg+="非法点火   ";
	}
    /* 第一字节*/
	b = szState.GetAt(0);
	b=Getmembyte(b);
	if(  (b &  0x2 )==0)
	{
		gpsData.iState= GPS_STATONE;
	}
	/*
	if(  (b &  0x4 )==0)
	{//H2
		gpsData.iState= GPS_STATTWO;
	}
	if(  (b &  0x8 )==0)
	{//L1
		gpsData.iState= GPS_STATTHREE;
	}
	*/
	b = szState.GetAt(6);
	b=Getmembyte(b);
	
	if(  (b &  0x1 )==0  || (b &  0x8 )==0 )
	{
		gpsData.iState=GPS_REGIONALARM;
		return;
	}
	
	b = szState.GetAt(5);
	b=Getmembyte(b);
	if(  (b &  0x01 )==0)
	{
	}
	b = szState.GetAt(1);
	b=Getmembyte(b);
	if(  (b   & 0x01 )==0)
	{
		
	}
	//掉电
	b = szState.GetAt(3);
	b=Getmembyte(b);
	if(  (b   & 0x08 )==0)
	{
		gpsData.iState=GPS_NOelectricity;
		return;
	}	
	//gps开路
	b = szState.GetAt(2);
	b=Getmembyte(b);
	if(  (b   & 0x02 )==0)
	{
		//		gpsData.iState=GPS_NOGPSINFO ;
	}
	/*
	if((b   & 0x08 )==0)
	{//L2
		gpsData.iState= GPS_STATFOR;
	}
	b = szState.GetAt(5);
	b=Getmembyte(b);
	if(  (b   & 0x01 )==0)
	{
		gpsData.iState=GPS_DOOR ; //车门
	}
	//GPS_ACC
	if(  (b   & 0x04 )==0x04)
	{
		gpsData.iState=GPS_ACC ; //ACC开
	}
	//
	//
	b = szState.GetAt(4);
	b=Getmembyte(b);
	if(  (b   & 0x02 )==0)
	{
		gpsData.iState=GPS_POWER ; //发动机
	}
	*/
	return;

}
//用来设置设备的状态的函数;
void CGTQEquipment::SetUDPSata(CString szState, CGPSData &gpsData)
{
	gpsData.iState=0;
	if(szState.GetLength()<8)
		return;
	byte  b=0;	
	b=szState.GetAt(7);//报警位;
	b=Getmembyte(b);
	if((b & 0x2)==0)
	{
		// 
		gpsData.iState=GPS_ROBALARM;
		return;
	}
	
	//	b=szState.GetAt(4);
	if((b & 0x4)==0)
		//	if((b &  0x08)==0)
	{
	//	gpsData.iState=GPS_HIGHSPEEDALARM;
		//发送一个报警解除
//		CString  szText="*HQ,000,R7,121212#";
	//	SendTQGPSCommandStr(gpsData.szEquimentID,szText);
		//end
	}
	
	b=szState.GetAt(7);
	if(  (b &  0x8 )==0)
	{
		//		LocationInfo.ulStatus |= 0x00000002;  //非法点火;
		
		//		szMsg+="非法点火   ";
	}
    /* 第一字节*/
	b = szState.GetAt(0);
	b=Getmembyte(b);
	if(  (b &  0x2 )==0)
	{
		gpsData.iState= GPS_STATONE;
	}
	b = szState.GetAt(6);
	b=Getmembyte(b);
	
	if(  (b &  0x1 )==0  || (b &  0x8 )==0 )
	{
//		gpsData.iState=GPS_REGIONALARM;
	}
	
	b = szState.GetAt(5);
	b=Getmembyte(b);
	if(  (b &  0x01 )==0)
	{
	}
	b = szState.GetAt(1);
	b=Getmembyte(b);
	if(  (b   & 0x01 )==0)
	{
		
	}
	//掉电
	b = szState.GetAt(3);
	b=Getmembyte(b);
	if(  (b   & 0x08 )==0)
	{
		gpsData.iState=GPS_NOelectricity;
	}	
	//gps开路
	b = szState.GetAt(2);
	b=Getmembyte(b);
	if(  (b   & 0x02 )==0)
	{
		//		gpsData.iState=GPS_NOGPSINFO ;
	}	
	return;
	
}

byte CGTQEquipment::Getmembyte(byte &b)
{
	if((b>0x2F) &&(b<0x3A))
		b=b-(0x30);
	if((b>0x40) &&(b<0x47))
		b=b-(0x37);
	return   b;
}

//得到时间gps时间;
COleDateTime  CGTQEquipment::GetNOYSTime(const CString  &szData)
{
	//取得tcp的时间;
	COleDateTime  now1=COleDateTime::GetCurrentTime();
	CString  szTime;
	int  iIndex=0;
	iIndex=szData.Find(TEXT("V1"));
	if(iIndex==-1)
	{
		iIndex=szData.Find(TEXT("V4"));
		if(iIndex==-1)
			return  now1;
		szTime=szData.Mid(iIndex+13,6);
	}
	else
	{
		szTime=szData.Mid(iIndex+3,6);
	}	
	CString  szDate;
	iIndex=szData.Find(TEXT("#"));
	if(iIndex==-1)
	{		
		return  now1;
	}
	szDate=szData.Mid(iIndex-15,6);  //日期;
	CString  szHH=szTime.Mid(0,2);
	//加入16
	int  iHH=atoi(szHH.GetBuffer(0));
	if(iHH<=15)  
	{//正常的处理;
		CString  sztemp;
		sztemp.Format("20%s-%s-%s %d:%s:%s",szDate.Mid(4,2),szDate.Mid(2,2),szDate.Mid(0,2),atoi(szTime.Mid(0,2))+8,szTime.Mid(2,2),szTime.Mid(4,2));
		COleDateTime  now;
		now.ParseDateTime(sztemp);
		return  now;	
	}
	if(iHH>=16)  //计算第二天的；
	{
		CString  sztemp;
		CString  szTemptime=szTime.Mid(2,4);
		szTime.Format("%02d%s",iHH+8-24,szTemptime);
		COleDateTimeSpan  Span(1,0,0,0);		
		sztemp.Format("20%s-%s-%s %d:%s:%s",szDate.Mid(4,2),szDate.Mid(2,2),szDate.Mid(0,2),atoi(szTime.Mid(0,2)),szTime.Mid(2,2),szTime.Mid(4,2));
		COleDateTime  now;
		now.ParseDateTime(sztemp);
		now+=Span;
		return  now;			
	}
	return  now1;
}


COleDateTime  CGTQEquipment::GetYSTime(const CString  &szData)
{	//取得udp的时间;
	COleDateTime  now1=COleDateTime::GetCurrentTime();
	CString  szTime;	
	szTime=szData.Mid(11,6);
	CString  szHH=szTime.Mid(0,2);
	//加入16
	int  iHH=atoi(szHH.GetBuffer(0));
	if(iHH<=15)
	{//正常的处理;
		CString  szDate;
		szDate=szData.Mid(17,6);
		CString  sztemp;
		sztemp.Format("20%s-%s-%s %d:%s:%s",szDate.Mid(4,2),szDate.Mid(2,2),szDate.Mid(0,2),atoi(szTime.Mid(0,2))+8,szTime.Mid(2,2),szTime.Mid(4,2));
		COleDateTime  now;
		now.ParseDateTime(sztemp);
		return  now;	
	}
	if(iHH>=16)  //计算第二天的；
	{
		CString  szDate;
		szDate=szData.Mid(17,6);
		CString  sztemp;
		CString  szTemptime=szTime.Mid(2,4);
		szTime.Format("0%d%s",iHH+8-24,szTemptime);
		COleDateTimeSpan  Span(1,0,0,0);
		
		
		sztemp.Format("20%s-%s-%s %d:%s:%s",szDate.Mid(4,2),szDate.Mid(2,2),szDate.Mid(0,2),atoi(szTime.Mid(0,2)),szTime.Mid(2,2),szTime.Mid(4,2));
		COleDateTime  now;
		now.ParseDateTime(sztemp);
		now+=Span;
		//		sztemp.Format("%s",now.Format("%Y-%m-%d %H:%M:%S"));
		return  now;	
		
	}
	return now1;
	
	
}




//处理所有的命令的函数;
BOOL CGTQEquipment::SendTQGPSCommandStr(CString szEquimentKey, CString szStr)
{
	CString  szLog;
	szLog.Format("CMTGPSEquipment::SendTQGPSCommand begin  %s  ",szEquimentKey);
	char  logbuf[1000]={0}; 
	sprintf(logbuf,"%s",szLog);
	__GTQSendGpsLog(logbuf);
	EquiNetInfo *pNetInfo=this->m_EquimenList.Find(szEquimentKey);
	//发送命令
	if(!pNetInfo)
	{
		szLog="CGTQEquipment::SendMTGPSCommand pNetInfo  ==NULL  return  FALSE ";
		memset( logbuf,0,1000);
		sprintf(logbuf,"%s",szLog);
		__GTQSendGpsLog(logbuf);
		return FALSE;
	}
	
	struct  sockaddr_in   addr=pNetInfo->addr;
	// addr.sin_port=htons(addr.sin_port);
	int   fd=pNetInfo->fd;
	if(fd==INVALID_SOCKET)
	{
		szLog="fd==INVALID_SOCKET  return  FALSE ";
		memset( logbuf,0,1000);
		sprintf(logbuf,"%s",szLog);
		__GTQSendGpsLog(logbuf);
		return  FALSE;
	}
	char bufcmd[1024]={0};
    sprintf(bufcmd,"%s",szStr);
    int i=send(fd,bufcmd,szStr.GetLength(),0);
	
	if(i<=0)
	{
		szLog="i<=0  return  FALSE ";
		memset( logbuf,0,1000);
		sprintf(logbuf,"%s",szLog);
		__GTQSendGpsLog(logbuf);
		
		return FALSE;
	}
	int irroe=WSAGetLastError();
	//en
	__GTQSendGpsLog(bufcmd);
	return  TRUE;
}

// SocketService.cpp: implementation of the CSocketService class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UDPSocketService.h"
//#include "Net.h"
//
//#include "MemoryPool.h"

#include "TQEquipment.h"
#include "DBWork.h"

HANDLE hExitEvent = 0;
extern CDBWork  g_DBWork;

//CString   szReType2="I1,";  //需要确认的晖映信息
//CString   szReType3="";  //需要确认的晖映信息

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//extern  int CreateTcpSocketServer(const  char  *szip,const short iPort);
//extern  void ReleaseSocket(int  fd);
//
//CMutex    g_MemPoolMutexLock(FALSE,"MemPoolMutexLock");
//CMemoryPool   g_MemPool;  //用的一个线程池;

double DecLONG(char *pLONG)
{
	double   Dlong=atof(pLONG);
	int  ilong=(int)(Dlong/100);
	return ilong+(Dlong-ilong*100)/60;	  
}

double DecLat(char *pLat)
{
	double   DLat=atof(pLat);
	int  iLat=(int)(DLat/100);
	return iLat+(DLat-iLat*100)/60;

}

UINT SetState(char *pState)
{
	return  0;

}

COleDateTime  GetNOYSTime(const CString  &szData)
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
		szTime=szData.Mid(iIndex+14,6);
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
	{
		//正常的处理;
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
		szTime.Format("0%d%s",iHH+8-24,szTemptime);
		COleDateTimeSpan  Span(1,0,0,0);		
		sztemp.Format("20%s-%s-%s %d:%s:%s",szDate.Mid(4,2),szDate.Mid(2,2),szDate.Mid(0,2),atoi(szTime.Mid(0,2)),szTime.Mid(2,2),szTime.Mid(4,2));
		COleDateTime  now;
		now.ParseDateTime(sztemp);
		now+=Span;
		return  now;			
	}

	return  now1;
}

COleDateTime  GetYSTime(const CString  &szData)
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
		if (now.GetStatus() == COleDateTime::valid)
		{
			now += Span;
		}
		else
		{
			now = COleDateTime::GetCurrentTime();
			now += Span;
		}
		
		//sztemp.Format("%s",now.Format("%Y-%m-%d %H:%M:%S"));
		return  now;	

	}

	return now1;
}

//判断数据的类型;
TQDATATYPE QueryDataType(char* p, int ilen, int& SubTypeLen)
{
	if(!p)
		return TYPE_TQNULL; 
	if(p[0]=='$')
		return  TYPE_PRESS;   //压缩数据的标志;
	SubTypeLen=0;
	if(p[15]=='V'  && p[16]=='1')
		return  TYPE_V1;
	if(p[15]=='I'  && p[16]=='1')
		return  TYPE_I1;
	if(p[15]=='I'  && p[16]=='8')
		return  TYPE_I8;
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
		return  TYPE_V4;

	}
	return TYPE_TQNULL;

}

//BCD串转化成ASc串
BOOL GetDCBTOASC(char *buf, int iLen, CString &szStr)
{
	if(buf==NULL)
		return  FALSE;
	szStr.Empty();
	szStr.Format("%c",buf[0]);
	for(int  iLoop=1;iLoop<iLen;iLoop++)
	{
	//	char  c=buf[iLoop];
		BCDCHAR  *pBcd=(BCDCHAR *)(&buf[iLoop]);
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
//	_Log(szStr);
	return  TRUE;

}

// Our packet processing function
void WINAPI OnClientToServer(HighPerformanceUDPServer * server, 
	sockaddr_in & address, 
	UINT16 iLen, 
	UINT8 * pData)
{
	if(iLen == 0) return;

	// Show who this came from
	printf("[%i.%i.%i.%i:%i][%i bytes]\n", 
		address.sin_addr.S_un.S_un_b.s_b1, 
		address.sin_addr.S_un.S_un_b.s_b2, 
		address.sin_addr.S_un.S_un_b.s_b3, 
		address.sin_addr.S_un.S_un_b.s_b4, 
		ntohs(address.sin_port), iLen);

	///////////////////////////////////////////////////////
	if(!pData)
	{
		return;
	}

	CGPSData GpsData;

	TQV1Data *pTQV1Data=NULL;
	TQV42Data *pTQV42Data=NULL;
	TQV43Data *pTQV43Data=NULL;

	if(pData[0]=='*')
	{
		//未压缩的数据;
		CString  szPp=pData;
		int   SubTypeLen=0;
		TQDATATYPE iType=TYPE_V1;//QueryDataType(pData, p->iMsgLen,SubTypeLen);
		switch(iType) 
		{
		case  TYPE_V1:
			{//一般的数据信息;
				pTQV1Data=(TQV1Data * )(pData);
				pTQV1Data->init();
				GpsData.fLongitude=DecLONG(pTQV1Data->Longtinue);

				GpsData.fLatitude=DecLat(pTQV1Data->Lat);
				GpsData.fSpeed=(float)atof(pTQV1Data->Speed);
				GpsData.iDirection=atoi(pTQV1Data->Diredtion);
				GpsData.iState=SetState(pTQV1Data->state);
				if(pTQV1Data->Able[0]='A')
					GpsData.iFlag=6;
				else
				{
					GpsData.iFlag=0;
					return;
				}

				//侧测试的时候用的
				//COleDateTime  now=COleDateTime::GetCurrentTime();
				COleDateTime now=GetNOYSTime(szPp);
				CString szTime=now.Format("%Y-%m-%d %H:%M:%S");
				sprintf(GpsData.Time,"%s",szTime);
				//GpsData.Time[20]=0;
				//end;

				memcpy(GpsData.szEquimentID,pTQV1Data->szKey,10);  //得到设备的key;
				//if(this->GetTaxibeWork(GpsData.szEquimentID)==TRUE)
				//{
				//	GpsData.iState=GPS_STATONE;
				//}
			}
			break;
		case TYPE_V4:
			{
				if(SubTypeLen==2)
				{
					pTQV42Data=(TQV42Data * )(pData);
					pTQV42Data->init();
					CString szReType2 = "I1,";
					if(szReType2.Find(pTQV42Data->TYPE2)==-1)
						return;  //不是一开始定义的就不用理会;

					GpsData.fLongitude=DecLONG(pTQV42Data->Longtinue);

					GpsData.fLatitude=DecLat(pTQV42Data->Lat);
					GpsData.fSpeed=(float)atof(pTQV42Data->Speed);
					GpsData.iDirection=atoi(pTQV42Data->Diredtion);
					GpsData.iState=SetState(pTQV42Data->state);
					if(pTQV42Data->Able[0]='A')
						GpsData.iFlag=6;
					else
					{
						return;
						GpsData.iFlag=0; 
					}
					//侧测试的时候用的
					//   COleDateTime  now=COleDateTime::GetCurrentTime();
					COleDateTime now=GetNOYSTime(szPp);
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
					pTQV43Data=(TQV43Data * )(pData);
					pTQV43Data->init();
				}
				// 设备回应的信息

			}
			break;
		//case  TYPE_I1:
		//	{
		//		//短消息
		//		DOTQMessage(pData,p->iMsgLen,szMessage);
		//		memcpy(GpsData.szEquimentID,pData+4,10);  //得到设备的key;
		//		GpsData.iFlag=0;
		//		COleDateTime  now=COleDateTime::GetCurrentTime();
		//		CString     szTime=now.Format("%Y-%m-%d %H:%M:%S");
		//		sprintf(GpsData.Time,"%s",szTime);
		//	}
		//	break;

		//case  TYPE_I8:
		//	{ //从计价器发过来的信息;

		//		CString  szStr;
		//		if(GetDCBTOASC(pData,p->iMsgLen,szStr)==FALSE)
		//			return  FALSE;
		//		memcpy(GpsData.szEquimentID,pData+4,10);  //得到设备的key;
		//		szMessage=GetTaxiData(GpsData.szEquimentID,szStr);
		//		GpsData.iFlag=0;
		//		COleDateTime  now=COleDateTime::GetCurrentTime();
		//		CString     szTime=now.Format("%Y-%m-%d %H:%M:%S");
		//		sprintf(GpsData.Time,"%s",szTime);
		//		//装客
		//		if(szMessage.Find("重车")!=-1)  
		//		{
		//			this->SetTaxistate(GpsData.szEquimentID,TRUE);
		//		}
		//		//下客了;
		//		if(szMessage.Find("空车")!=-1  ||  szMessage.Find("交易信息")!=-1)
		//		{				  
		//			this->SetTaxistate(GpsData.szEquimentID,FALSE);
		//		}
		//	}
		//	break;
		default:
			ASSERT(FALSE);
			return ;
		}	   
	}
	//$3070320141084211300407  3116853500 121278339E000168FBFFFBFFFF0048
	if(pData[0]=='$')
	{
		//压缩的数据;
		TQPREDATA  *pPreData=NULL;
		pPreData=(TQPREDATA*)(pData);
		if(pPreData==NULL)
			return ;

		CString  szStr;
		if(GetDCBTOASC((char*)pData, iLen, szStr) == FALSE)
			return;

		int iStrLen = szStr.GetLength();
		if(iStrLen < 60)
			return ;
		CString szKey=szStr.Mid(1,10);
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
			return;
			GpsData.iFlag=0; 
		}

		//侧测试的时候用的
		//COleDateTime  now=COleDateTime::GetCurrentTime();
		COleDateTime now = GetYSTime(szStr);
		CString szTime=now.Format("%Y-%m-%d %H:%M:%S");
		sprintf(GpsData.Time,"%s",szTime);   

		CString szEquimentID;
		if(g_DBWork.GetEquimentInfoID(szKey, szEquimentID, TRUE) < 1)
		{	
			return;
		}

		ZeroMemory(GpsData.szEquimentID, 15);
		sprintf(GpsData.szEquimentID, "%s", szEquimentID);
		GpsData.szEquimentID[szEquimentID.GetLength()]=0;

		//if(this->GetTaxibeWork(GpsData.szEquimentID)==TRUE)
		//{
		//	GpsData.iState=GPS_STATONE;
		//}
	}

	//保存数据库
	COleDateTime nowTime;
	nowTime.ParseDateTime(GpsData.Time);
	if(nowTime.GetStatus()==COleDateTime::valid)
	{
		CString szTable;
		szTable.Format("GPSDATA%s", nowTime.Format("%Y%m%d"));	
		//g_DBWork.CreateTable(szTable);
		//szTable="GPSDATA";
    	g_DBWork.UpdataGPSData(szTable, GpsData);
		g_DBWork.UpdataGPSNewestData(GpsData);  //最好能加个时间比较的  20070712
	}

}

CUDPSocketService::CUDPSocketService()
{
	

}

CUDPSocketService::~CUDPSocketService()
{

}

BOOL CUDPSocketService::StartService()
{
	// Setup the library
	if(HighPerformanceUDPServer::Initialize() == false)
	{
		return FALSE;
	}

	// Create our server using only 1 IOCP thread and have 2 worker 
	// threads available. We want to use the default number of pending 
	// receives for the OS.
	if(m_HPUDPServer.Create(4254, 1, 2, 0, 0) == false)
	{
		HighPerformanceUDPServer::Deinitialize();
		return FALSE;
	}

	// The server will invoke the following function on an event.
	m_HPUDPServer.SetClientToServerCallback(OnClientToServer);

	// So we know when to exit
	//hExitEvent = CreateEvent(0, TRUE, FALSE, 0);

	// Set a console handler to make our program more flexible and simple.
	// We will be  able to Ctrl + C out of our program rather than have to
	// code in a more complicated GUI.
	//SetConsoleCtrlHandler(ConsoleHandler, TRUE);

	// Simply wait for the user to close the server. For this example,
	// we are wasting our main thread, but we can work in additional logic
	// so this is not the case later on.
	//WaitForSingleObject(hExitEvent, INFINITE);

	// Remove the console handler as we no longer need it.
	//SetConsoleCtrlHandler(ConsoleHandler, FALSE);

	// Clean up the server itself
	//m_HPUDPServer.Destroy();

	// Clean up the library
	//HighPerformanceUDPServer::Deinitialize();

	// Free the resource
	//CloseHandle(hExitEvent);


	// Standard return
	return TRUE;

	//CSystemInfo   systenifo;
	//short  iPort=systenifo.GetPort();
	//fd=CreateTcpSocketServer(NULL,iPort);
	//if(fd>0)
	//{ 

 //       BOOL bR=m_ReadThread.Start(fd);
	//    BOOL bW=m_WorkThread.Start();
	//	if(bR &&  bW)
	//		return  TRUE;
	//}
	//return  FALSE;
}


int CUDPSocketService::EndService()
{
	// Clean up the server itself
	m_HPUDPServer.Destroy();

	// Clean up the library
	HighPerformanceUDPServer::Deinitialize();


	//ReleaseSocket(fd);
	//m_ReadThread.Stop();
	//m_WorkThread.Stop();

	//g_MemPool.ReleasePool();

	return  1;
}





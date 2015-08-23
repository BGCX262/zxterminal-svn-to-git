// BaseNetAgreement.cpp: implementation of the CBaseNetAgreement class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetServer.h"
#include "BaseNetAgreement.h"
#include "Net.h"
#include "Log.h"
#include "MemoryPool.h"
#include <afxmt.h>
#include "DBWork.h"
#include "Resource.h"
#include "NetField.h"
#include "GuardFiled.h"
#include "Systeminfo.h"
#include "DoUserEvent.h"
#include "fstream"
using namespace  std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern  int SendToNetMsg(int  fd,char *pBuf,int iLen);
extern  void      g_Log(CString szLog);
extern  CMemoryPool   g_MemPool;  //用的一个线程池;
extern  CMutex    g_MemPoolMutexLock;

extern  CDBWork g_DBWork;
extern  CNetServerApp   theApp;
extern  CMutex  g_ClientManagerLock;
#define MSG_HAVEUSERLOGON  -2


void   __SendBaseNetLog(CString  szLog)
{
	char  buf[500]={0};
	sprintf(buf,"%s",szLog);
	ofstream  f("BaseNet.Log",ios::app| ios::out);
	f<<buf<<endl;
	f.close();
}

CBaseNetAgreement::CBaseNetAgreement()
{
	m_pBaseEquipment=NULL;
	m_pMTJJAEquipment=NULL;
	m_pMTGPSEquipment=NULL;
	m_pTQEquipment=NULL;

}

CBaseNetAgreement::~CBaseNetAgreement()
{
	m_pBaseEquipment=NULL;
	m_pMTJJAEquipment=NULL;
	m_pMTGPSEquipment=NULL;
	m_pTQEquipment=NULL;

}

CBaseNetAgreement::CBaseNetAgreement(int fd, NetMsgHead *p, int iMsgLen)
{
	m_MsgLen=iMsgLen;
	m_pThreadMsg=p;
	m_fd=fd;
	m_pBufMsg=NULL;
	m_pBaseEquipment=NULL;
	m_pMTJJAEquipment=NULL;
	m_pMTGPSEquipment=NULL;
	m_pTQEquipment=NULL;

}
void CBaseNetAgreement::Clear()
{
	m_pBufMsg = NULL;
}

/*把网络数据打包成一个Net数据包，并且返回数据包的长度，-1，失败；*/
int CBaseNetAgreement::PackNetMsg(const BYTE BMsgType, MIANMSGTYPE MMsgType, SUBMSGTYPE SubType, char* pData, int iDateLen)
{
	if(!pData  && iDateLen!=0 )
	{
		return  -1; //数据包中的数据体的指针为0
	}

	NetMsgHead temp;
	temp.MsgType=BMsgType;
	temp.Version=htonl(temp.Version);  //added 20060820
	temp.MainMsgType=(MIANMSGTYPE)htonl(MMsgType);
	temp.SubMsgType=(SUBMSGTYPE)htonl(SubType);
	temp.NetMsgLen=htonl(sizeof(NetMsgHead)+ iDateLen);
	temp.ContentLen=htonl(iDateLen);
	char  *pMsg=NULL;
	if((sizeof(NetMsgHead)+ iDateLen)>POOLBUFLEN)
	{//内存池的大小小于消息的长度
		pMsg=new  char [sizeof(NetMsgHead)+ iDateLen];
	}
	else
	{	g_MemPoolMutexLock.Lock();
		pMsg=g_MemPool.GetBuf();
		g_MemPoolMutexLock.Unlock();
	}
	if(!pMsg)
		return  -1;
	memcpy(pMsg,&temp,sizeof(NetMsgHead));
	memcpy(pMsg+sizeof(NetMsgHead),pData,iDateLen);
	this->m_pBufMsg=pMsg;
	return  sizeof(NetMsgHead)+ iDateLen;
}

void CBaseNetAgreement::DoCommMsg()
{
	if(!m_pThreadMsg)
	{
		return;
	}

	if(m_pThreadMsg->MsgType == MSGTYPE_COMMON)
	{
		//公共的和设备无关的信息；
		switch(m_pThreadMsg->MainMsgType)
		{
		case MAINMSG_USERLOGIN:  //用户登陆;
			{
				UserLogin* pUserLogin = (UserLogin*)((char *)m_pThreadMsg + (sizeof(NetMsgHead)));
				SUBMSGTYPE itype = SUBMSG_NULL;
				int iRet = IsLegalUser(pUserLogin);
				if(iRet == 1)
				{
					g_ClientManagerLock.Lock();
					theApp.m_Clientmanager.AddNode(this->m_fd,0,"",pUserLogin->szUserId);  //用用户管理;
					g_ClientManagerLock.Unlock();

					theApp.m_pMainWnd->SendMessage(WM_CONNECT, this->m_fd, 1);					

					//发送到界面进行处理;
					itype = SUBMSG_REQUES_SUCESSFUL;				
				}
				else
				{
					theApp.m_Clientmanager.DeletNode(this->m_fd);
					if(iRet == MSG_HAVEUSERLOGON)
					{
						itype=SUBMSG_REQUES_USERHAVELONGIN;
					}
					else
					{
					   itype=SUBMSG_REQUES_ERROR;
					}
				}

				CString   szLog;
				szLog.Format("UserId=%s的用户登陆%d", pUserLogin->szUserId, (int)itype);
				g_Log(szLog);

				SendToNetMessage(MSGTYPE_COMMON, MAINMSG_USERLOGIN, itype, this->m_pBufMsg, 0);			
				//DoUserPowerInfo(pUserLogin->szUserId);
				//DoSoftVersionInfo();
				break;
			}
		case MAINMSG_USERALLEQUIID:
			{
				if(m_pThreadMsg->SubMsgType == SUBMSG_REQUES)
				{
					//请求用户的所有设备 id;
					UserLogin* pUserLogin = (UserLogin*)((char*)m_pThreadMsg + (sizeof(NetMsgHead)));
					RestoreUserAllEquiment(pUserLogin->szUserId);					
				}
			}
			break;
		case MAINMSG_EQUIMENTINFO:
			{
				if(m_pThreadMsg->SubMsgType == SUBMSG_REQUES)
				{
					//请求设备的信息
                    CQueryEquimnetInfo  *pQueryInfo = (CQueryEquimnetInfo  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
					if(!pQueryInfo)
					{
						break;
					}

					RestoreEquimentInfo(pQueryInfo->m_EquimentId);
				}
			}
			break;
/*//delete   20061107
			case MAINMSG_HEARTBEAT:
				{
					if(m_pThreadMsg->SubMsgType==SUBMSG_REQUES)
					{//请求设备的信息
			            HeartBeat  *pHeartBeat=(HeartBeat  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
						if(!pHeartBeat)
							break;
						RestoreHeartBeat();
					}
				}
				break;*/
		case MAINMSG_DATADOWNLOAD  ://数据下载
			{
				CDownLoadCommand  *pDataCommand=(CDownLoadCommand  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
                DoDataDownLaod(pDataCommand);
				break;
			}
			//20070226
		case MAINMSG_CHANGEUSERPWS  ://更改密码;
			{
				CChangePwsDate  *pChangePwsDate=(CChangePwsDate  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
                
                DoChangePwsDate(pChangePwsDate);
				break;
			}			
		case  MAINMSG_CHANGEUSERVALIDATE :  //用户验证号更改
			{
				CChangeValidDate  *pChangeValidDate=(CChangeValidDate  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
                DoChangeValidDate(pChangeValidDate);
				break;
			}
		case  MAINMSG_CLIENTQUERY:  //客户查询;
			{
				CUserQueryEquimentInfo  *pUserQueryEquimentInfo=(CUserQueryEquimentInfo  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
                DoUserQueryEquimentInfo(pUserQueryEquimentInfo);
				break;
			}
		case  MAINMSG_USERPOWER: //权限请求;
			{
				Userpower   *pUserpower=(Userpower  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
				if(pUserpower)
				{
					CString szUser=pUserpower->szUserId;
					DoUserPowerInfo(szUser);
				}

				break;
			}
		case  MAINMSG_SOFTVERSION:   //软件版本请求;
			{
				DoSoftVersionInfo();
				break;
			}

		case  MAINMSG_OPATERRESULT://  处理结果
			{
				COpeateResult   *pOpeateResult=(COpeateResult  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
				if(pOpeateResult)
				{
					DoOpeateResult(pOpeateResult);
				}

			}
			break;
		case MAINMSG_INFORMCLIENT:   ////通知客户 和任务安排  //DoMessageToClient
			{
				CMessageToClient   *pMessageToClient=(CMessageToClient  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
				if(pMessageToClient)
				{
					DoMessageToClient(pMessageToClient);
				}
				break;
			}
			
		case MAINMSG_CTRTELDATEDOWN://控制手机信息下载
			{
				CCtrTels   *pCtrTels=(CCtrTels  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
				if(pCtrTels)
				{
					DoCtrTels(pCtrTels);
				}

			}
			break;
		case  MAINMSG_SENDTELDATEDOWN:   //报警接受手机下载
			{
				CAlarmRecvTels* pAlarmRecvTels = (CAlarmRecvTels*)((char*)m_pThreadMsg + (sizeof(NetMsgHead)));
				if(pAlarmRecvTels)
				{
					DoDownLaodAlarmRecvTels(pAlarmRecvTels);
				}
			}
			break;
		case MAINMSG_FEEINFODATEDOWN:  //费用信息下载
			{

				CFeeInfo   *pFeeInfo=(CFeeInfo  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
				if(pFeeInfo)
				{
					DoDownLaodFeeInfo(pFeeInfo);
				}
			}
			break;
		case   MAINMSG_ALARMINFODATEDOWN :   //报警信息描述下载;
			{
				
				CGpsAlarmInfo   *pGpsAlarmInfo=(CGpsAlarmInfo  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
				if(pGpsAlarmInfo)
				{
					DoDownLaodGpsAlarmInfo(pGpsAlarmInfo);
				}
			}
			break;
		case MAINMSG_UPDATESENDTEL: //更新报警接收手机
			{
				CAlarmRecvTels   *pAlarmRecvTels=(CAlarmRecvTels  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
				if(pAlarmRecvTels)
				{
					DoUpdateAlarmRecvTels(pAlarmRecvTels);
				}

			}
			break;
		case  MAINMSG_UPDATECTRTEL:  // 更新控制手机信息
			{
				CCtrTels   *pCtrTels=(CCtrTels  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
				if(pCtrTels)
				{
					DoUpDateCtrTels(pCtrTels);
				}
			}
			break; 
		case  MAINMSG_UPDATEALARMINFO:  //更新报警描述;
			{
				
				CGpsAlarmInfo   *pGpsAlarmInfo=(CGpsAlarmInfo  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
				if(pGpsAlarmInfo)
				{
					DoUpDatepsAlarmInfo(pGpsAlarmInfo);
				}
			}
			break;
		case MAINMSG_DOWNLOADMAP:   //下载用户的可以地图列表;
			{

				CUserMapInfo   *pUserMapInfo=(CUserMapInfo  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
				if(pUserMapInfo)
				{
					DoDownLoadUserMapList(pUserMapInfo);
				}

			}
			break;
		case  MAINMSG_HEARTBEAT:  //心跳测试   added  by  20070325
			{				
				DoHeartBeat(); 
			}
			break;
			
		case  MAINMSG_GPSDATAQUERY:  //gps数据请求  added  by  200700704
			{	
				CGpsDataQuery   *pGpsDataQuery=(CGpsDataQuery  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
				
				if(pGpsDataQuery)
				{
			    	DoGpsDateQuery(pGpsDataQuery); 
				}
			}
			break;
		case MAINMSG_QUERYUSERNEWESTGPSDATA: //请求用户最新的gps数据;
			{
				CQueryNewestGpsData* pQueryNewestGpsData = (CQueryNewestGpsData*)((char*)m_pThreadMsg + (sizeof(NetMsgHead)));
				 
				if(pQueryNewestGpsData)
				{
					DoQueryUserGpsData(pQueryNewestGpsData);
				}
				break;
			}
			break;
		case  MAINMSG_GOOGLEMAPQUERY:  //用户请求卫星地图;
			{
				QueryGoogleMap* pQueryGoogleMap = (QueryGoogleMap*)((char*)m_pThreadMsg + (sizeof(NetMsgHead)));
				if(pQueryGoogleMap)
				{
					DoQueryGoogleMap(pQueryGoogleMap);
				}
				break;
			}
			break;
		default:
			break;
		}
	}
}
int CBaseNetAgreement::SendToNetMessage(const  BYTE  BMsgType, MIANMSGTYPE  MMsgType,SUBMSGTYPE  SubType,char  *pData,int iDateLen)
{
	Clear();
	int iToLen = this->PackNetMsg(BMsgType, MMsgType, SubType, pData, iDateLen);
	int iRet = SendToNetMsg(this->m_fd, this->m_pBufMsg, iToLen);
	if(iToLen > POOLBUFLEN)
	{
		delete []m_pBufMsg;
	}
	else
	{
		g_MemPoolMutexLock.Lock();	
		g_MemPool.ReleaseBuf(m_pBufMsg);
		g_MemPoolMutexLock.Unlock();
	}

	return  iRet;
}

int CBaseNetAgreement::IsLegalUser(const UserLogin *pUserLogin)
{

	if(g_DBWork.IsUserlegal(pUserLogin->szUserId,pUserLogin->szPWS))
	{
		//added  相同用户的检测
		g_ClientManagerLock.Lock();
		int fd=-1;
		theApp.m_Clientmanager.GetClientofFd(pUserLogin->szUserId,fd);  //检测是不是有拥护登陆了;

		g_ClientManagerLock.Unlock();
		if(fd!=-1)
			return MSG_HAVEUSERLOGON;
		return  1;
	}
	else
		return  0;
}


int CBaseNetAgreement::RestoreUserAllEquiment(CString  szUser)
{

	CString   szLog;
	szLog.Format("CBaseNetAgreement::RestoreUserAllEquiment()  begin szUserId=%s",szUser);
	g_Log(szLog);

	CString   szAll;
	CStringArray asUserAllEquiment;
	g_DBWork.GetUserAllEquiment(szUser,asUserAllEquiment);
	for(int i=0;i<asUserAllEquiment.GetSize();i++)
	{
		szAll+=asUserAllEquiment[i];		
		szAll+="&";
	}
	CUserAllEquiment   userallequi(szAll);
	int iType=-1;
	if(userallequi.iLen!=0)
	{
		iType=SUBMSG_REQUES_SUCESSFUL;
	}
	else
		iType=SUBMSG_REQUES_ERROR;
	 int iRet=	SendToNetMessage(MSGTYPE_COMMON,MAINMSG_USERALLEQUIID,(SUBMSGTYPE)iType,userallequi.pAllEqui,userallequi.iLen);			
     
	 szLog.Format("CBaseNetAgreement::RestoreUserAllEquiment()  end iRet=%d",iRet);
	 g_Log(szLog);
	 return  iRet;
}

int CBaseNetAgreement::RestoreEquimentInfo(CString EquimentId)
{
	CString   szLog;
	szLog.Format("CBaseNetAgreement::RestoreEquimentInfo()  begin EquimentId=%s",EquimentId);
	g_Log(szLog);
	if(EquimentId.IsEmpty())
		return -1;
	EquimentId.TrimLeft();
	if(EquimentId.GetLength()<EQUIMENTIDLEN)
		return -1;
	EquimentId=EquimentId.Mid(0,EQUIMENTIDLEN);
	CEquimentInfo  EquiInfo(EquimentId);
	int iType=-1;
	if(g_DBWork.GetEquimentInfo(EquimentId,EquiInfo)>0)
	{
		iType=SUBMSG_REQUES_SUCESSFUL;
	}
   else
   {
	   iType=SUBMSG_REQUES_ERROR;
   }
    
	int iRet=this->SendToNetMessage(MSGTYPE_COMMON,MAINMSG_EQUIMENTINFO,(SUBMSGTYPE)iType,(char *)&EquiInfo,sizeof(EquiInfo));	
  
   szLog.Format("CBaseNetAgreement::RestoreEquimentInfo()  end iRet=%d",iRet);
   g_Log(szLog);
   return  iRet;

}

void CBaseNetAgreement::DoGuardMsg()
{
	if(!m_pThreadMsg)
		return;

	if(m_pThreadMsg->MsgType==MSGTYPE_GUARD)
	{//公共的和设备无关的信息；
		CEquimentInfo  info;
		CGuardCommand  *pData=NULL;
		CResponCommandInfo  ResponCommandInfo;
		SUBMSGTYPE  SUBType=SUBMSG_COMMAND_ERROR;
		MIANMSGTYPE  mainType=m_pThreadMsg->MainMsgType;
		switch(mainType)
		{
		case   MAINMSG_GUARDCOMMAND:  //用户登陆;
			{
				
				pData=(CGuardCommand  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
				
				
				if(  !pData   ||  g_DBWork.GetEquimentInfo(pData->EquimentId,info)<0)
				{	
					return;
				}
				//分设备类型进行解析
                memcpy(ResponCommandInfo.EquimnetID,pData->EquimentId,15);
				switch(info.EquimentSubType)
				{
				case EQUIPMENT_MTJJA:
					{
						if(!m_pMTJJAEquipment)
							m_pMTJJAEquipment =new  CMTJJAEquipment;
					}
					if(m_pMTJJAEquipment)
					{
						m_pBaseEquipment=m_pMTJJAEquipment;
					}
				}
				//end;
				break;
			}			
		default:
			ASSERT(FALSE);
			break;
		}
		if(m_pBaseEquipment)
		{
			if(m_pBaseEquipment->SendCommand(info.EquimentKey,pData))
			{//发送回应信息到客户端;
				SUBType=SUBMSG_COMMAND_SUCESSFUL;
			}
			else
			{//
			}
		}
		else
		{
		}
		SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&ResponCommandInfo,sizeof(ResponCommandInfo));			
		
        return ;
	}
	

}


void CBaseNetAgreement::DoGpsMsg()
{
	CString  szLog="CBaseNetAgreement::DoGpsMsg()  begin";
	__SendBaseNetLog(szLog);
	if(!m_pThreadMsg)
	{
		szLog="CBaseNetAgreement::DoGpsMsg()  m_pThreadMsg==NULL  return ";
		__SendBaseNetLog(szLog);
		return;
	}
	
	if(m_pThreadMsg->MsgType==MSGTYPE_GPS)
	{
		CEquimentInfo  info;
		CGpsCommand  *pData=NULL;
		CResponCommandInfo  ResponCommandInfo;
		SUBMSGTYPE  SUBType=SUBMSG_COMMAND_ERROR;
		MIANMSGTYPE  mainType=m_pThreadMsg->MainMsgType;
		switch(mainType)
		{
		case   MAINMSG_GPSCOMMAND:  
			{
				szLog="CBaseNetAgreement::DoGpsMsg()  MAINMSG_GPSCOMMAND ";
				__SendBaseNetLog(szLog);
				
				pData=(CGpsCommand  *)((char *)m_pThreadMsg+(sizeof(NetMsgHead)));
				
				
				if(  !pData   ||  g_DBWork.GetEquimentInfo(pData->EquimentId,info)<0)
				{
					//要处理的;
					//		memcpy(ResponCommandInfo.EquimnetID,pData->EquimentId,15);
					//		SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&ResponCommandInfo,sizeof(ResponCommandInfo));			
					
					return;
				}
				//分设备类型进行解析
                memcpy(ResponCommandInfo.EquimnetID,pData->EquimentId,15);
				
				szLog.Format("CBaseNetAgreement::DoGpsMsg() %s",pData->EquimentId);   
				__SendBaseNetLog(szLog);
				switch(info.EquimentSubType)
				{
				case EQUIPMENT_MTGPS:
					{
						if(!m_pMTGPSEquipment)
							m_pMTGPSEquipment =new  CMTGPSEquipment;
					}
					if(m_pMTGPSEquipment)
					{
						//			m_pBaseEquipment=m_pMTGPSEquipment;
						if(m_pMTGPSEquipment->SendMTGPSCommand(info.EquimentKey,pData))
						{	//发送回应信息到客户端;
							SUBType=SUBMSG_COMMAND_SUCESSFUL;
						}
					}
					szLog.Format("CBaseNetAgreement::DoGpsMsg() m_pMTGPSEquipment==NULL");   
					__SendBaseNetLog(szLog);
					break;
				
				case EQUIPMENT_TQGPS:
					{
						if(!m_pTQEquipment)
							m_pTQEquipment =new  CTQEquipment;
						
						if(m_pTQEquipment)
						{
							//			m_pBaseEquipment=m_pMTGPSEquipment;
							if(m_pTQEquipment->SendTQGPSCommand(info.EquimentKey,pData))
							{	//发送回应信息到客户端;
								SUBType=SUBMSG_COMMAND_SUCESSFUL;
							}
						}
						break;
					}
					break;
					//20070606
				case EQUIPMENT_GTQGPS:
					{
						if(!m_pGTQEquipment)
							m_pGTQEquipment =new  CGTQEquipment;
						
						if(m_pGTQEquipment)
						{
						
							//			m_pBaseEquipment=m_pMTGPSEquipment;
							if(m_pGTQEquipment->SendTQGPSCommand(info.EquimentKey,pData))
							{	//发送回应信息到客户端;
								SUBType=SUBMSG_COMMAND_SUCESSFUL;
							}
						}
						break;
					}
					break;
				default:
					{
						ASSERT(FALSE);
						break;
					}
				//	break;
				}

			}
			break;
		default:
			{
				szLog.Format("CBaseNetAgreement::DoGpsMsg() NO type");   
	               __SendBaseNetLog(szLog);
				   ASSERT(FALSE);
				   break;
			}
		}
		SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&ResponCommandInfo,sizeof(ResponCommandInfo));			
		int   BSuccess=  SUBType== SUBMSG_COMMAND_SUCESSFUL ?    1:0;
		CDoUserEvent    aDoUserEvent(this->m_fd,pData->EquimentId,pData->iCOmmandID,pData->Param,BSuccess);
		aDoUserEvent.RecordUserEvent();
		return ;
      //  return ;
	}
		//added  20070505  增加用户对设备操作的纪录


	
	
}

void CBaseNetAgreement::DoDataDownLaod(CDownLoadCommand *pdata)
{

	CString   szLog;
	szLog.Format("CBaseNetAgreement::DoDataDownLaod()  begin ");
	g_Log(szLog);
	int iType=-1;
	CResponCommandInfo  ResponCommandInfo;
	CString  szDownLoadFileName;
	BOOL   bHaveDate=FALSE;
    
	if(pdata==NULL)
	{
		iType=SUBMSG_DATADOWNLOAD_ERROR;
	}
	else
	{
		//验证消息包的合法性;
		int  iCount=pdata->iCount;
		int  iEquimentType=pdata->iEquimentType;
		int  iDataType=pdata->iType;
	
		CString szBeginTiem=pdata->BeginTime;
		CString  szEndTime=pdata->EndTime;
		COleDateTime   OleBeginTime,OleEndTIme;
		OleEndTIme.ParseDateTime(szEndTime),OleBeginTime.ParseDateTime(szBeginTiem);
		BOOL  bBeginTimeValaue=(OleBeginTime.GetStatus()==COleDateTime::valid);
		BOOL  bEndTimeValaue=(OleEndTIme.GetStatus()==COleDateTime::valid);
		CString   szUserId=pdata->userId;
		szUserId.TrimLeft(),szUserId.TrimRight();
		int  idownloadnumber=pdata->DownLoadNumber;
		CDownLoadGpsDateResult   Redownlaod;
		Redownlaod.DownLoadNumber=idownloadnumber;

		if(iCount<=0 || iEquimentType>3 ||iEquimentType<1 ||  iDataType<0 ||iEquimentType>2  ||!bBeginTimeValaue ||!bEndTimeValaue  ||szUserId.IsEmpty())
		{//不合法的数据;
			iType=SUBMSG_DATADOWNLOAD_ERROR;
		}
		else
		{
			//合法的数据开始解析数据;
			//得到要下载的设备的表示;
			CStringArray  asEuqimentID;
			asEuqimentID.RemoveAll();
			asEuqimentID.SetSize(0);
			char  *paEuqimentId=pdata->asEquimentId;
			int indexStart=0;
			if(paEuqimentId==NULL)
				iType=SUBMSG_DATADOWNLOAD_ERROR;
			else
			{
				CString  szEuqimentid=paEuqimentId;
				CString   szTempId;
				for(int  iLoop=0;iLoop<iCount;iLoop++)
				{
					int index=szEuqimentid.Find(";");
					if(index==-1)
					{//没有找到
						break;
					}
					else
					{
						szTempId.Empty();
						szTempId=szEuqimentid.Mid(indexStart,index);
						if(!szTempId.IsEmpty())
						{
							asEuqimentID.Add(szTempId);
							indexStart=index+1;
							szDownLoadFileName+=szTempId;
						}
						else
						{
							//出现错误的空号
							iType=SUBMSG_DATADOWNLOAD_ERROR;
							break;
						}

					}
				}
				//找到后;
				//szUserId
				szDownLoadFileName.Empty();
				szDownLoadFileName.Format("%s_DownLoad%d.txt",szUserId,idownloadnumber);
			//	szDownLoadFileName+=".txt";
				CStdioFile f("gpsdatedownload\\"+szDownLoadFileName,CFile::modeCreate|CFile::modeReadWrite);
				if(asEuqimentID.GetSize()==iCount)
				{//开始下载数据
					if(iEquimentType==1)//gps
					{
						for(int  i=0;i<asEuqimentID.GetSize();i++)
						{
							CString szDownLoadEuqimentid=asEuqimentID.GetAt(i);
							if(szDownLoadEuqimentid.IsEmpty())
							{
								//发送信息到客户端;
						//		int iRet=	SendToNetMessage(MSGTYPE_COMMON,MAINMSG_USERALLEQUIID,(SUBMSGTYPE)iType,userallequi.pAllEqui,userallequi.iLen);

								continue;
							}
							TEMP_GPSDATA   asGpsData;
							asGpsData.RemoveAll();
							asGpsData.SetSize(0);
							if(g_DBWork.GetGpsData(szDownLoadEuqimentid,szBeginTiem,szEndTime,iDataType,asGpsData)<0)
							{
								iType=SUBMSG_DATADOWNLOAD_ERROR;
								sprintf(ResponCommandInfo.EquimnetID,"%s",szDownLoadEuqimentid);
								
								SendToNetMessage(MSGTYPE_COMMON,MAINMSG_DATADOWNLOAD,(SUBMSGTYPE)iType,(char *)&ResponCommandInfo,sizeof(ResponCommandInfo));
								
								//		int iRet=	SendToNetMessage(MSGTYPE_COMMON,MAINMSG_USERALLEQUIID,(SUBMSGTYPE)iType,userallequi.pAllEqui,userallequi.iLen);
								
								continue;
							}
							else
							{
								int  iDataCount=asGpsData.GetSize();
								if(iDataCount<=0)
								{
                                    
									iType=SUBMSG_DATADOWNLOAD_NORECORD;
									sprintf(ResponCommandInfo.EquimnetID,"%s",szDownLoadEuqimentid);
							
									SendToNetMessage(MSGTYPE_COMMON,MAINMSG_DATADOWNLOAD,(SUBMSGTYPE)iType,(char *)&ResponCommandInfo,sizeof(ResponCommandInfo));
									continue;

								}
								if(iDataCount>=1)
								{
									bHaveDate=TRUE;
								}
								for(int iLoop=0;iLoop<iDataCount;iLoop++)
								{
									CGPSData   date;
									date=asGpsData.GetAt(iLoop);
									CString  szLine;
									szLine.Format("%s,%s,%.8f,%.8f,%.5f,%d,%d,%d\n",date.szEquimentID,date.Time,date.fLongitude,date.fLatitude,date.fSpeed,date.iDirection,date.iState,date.iFlag);
								    f.WriteString(szLine);
									f.Flush();
								//	f.Close();
								}
							}
						}
						f.Close();
					//	if(bHaveDate)
						{
							Sleep(1000*2);
							iType=SUBMSG_DATADOWNLOAD_FINISHED;
							SendToNetMessage(MSGTYPE_COMMON,MAINMSG_DATADOWNLOAD,(SUBMSGTYPE)iType,(char *)&Redownlaod,sizeof(CDownLoadGpsDateResult));
						}

					}
					if(iDataType==2)//guard
					{

					}


				}
				else
				{
					iType=SUBMSG_DATADOWNLOAD_ERROR;
				}
			}
			
		}


	}
	//	int iRet=	SendToNetMessage(MSGTYPE_COMMON,MAINMSG_USERALLEQUIID,(SUBMSGTYPE)iType,userallequi.pAllEqui,userallequi.iLen);			
	
	szLog.Format("CBaseNetAgreement::DoDataDownLaod()  end");
	g_Log(szLog);
	return;
}

void CBaseNetAgreement::DoChangePwsDate(CChangePwsDate *pDate)
{
	CString  szLog="CBaseNetAgreement::DoChangePwsDate()  begin";
	__SendBaseNetLog(szLog);
	if(!pDate)
		return;
    CChangePwsDate  PwsDate;
	memcpy(&PwsDate,pDate,sizeof(CChangePwsDate));
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_ERROR;
	MIANMSGTYPE  mainType=MAINMSG_CHANGEUSERPWS;
	CString szUserID=pDate->szUserId;
	CString szOldPws=pDate->szOldPws;
	CString  szNewPws=pDate->szNewPws;
	CString  szTwoPws=pDate->sztwoPws;
	szUserID.TrimLeft(),szUserID.TrimRight();
	szOldPws.TrimLeft(),szOldPws.TrimRight();
	szNewPws.TrimLeft(),szNewPws.TrimRight();
	szTwoPws.TrimLeft(),szTwoPws.TrimRight();
	if(szNewPws!=szTwoPws ||szUserID.GetLength()>29 ||szOldPws.GetLength()>29 ||szTwoPws.GetLength()>29 )
	{
		 SUBType=SUBMSG_REQUES_ERROR;
	}
	else
	{//参数正确的时候;
		if(g_DBWork.IsUserlegal(szUserID,szOldPws)==FALSE)
			SUBType=SUBMSG_REQUES_ERROR;
		else
		{
			//如果提交的旧密码是正确的就修改密码;
			if(g_DBWork.UpdateUserPws(szUserID,szNewPws)<0)
			{
				SUBType=SUBMSG_REQUES_ERROR;
				
			}
			else
				SUBType=SUBMSG_REQUES_SUCESSFUL;
		}
	}
	

	SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&PwsDate,sizeof(PwsDate));			
		
     
	szLog="CBaseNetAgreement::DoChangePwsDate()  end";
    __SendBaseNetLog(szLog);
	 return ;
	

}

void CBaseNetAgreement::DoChangeValidDate(CChangeValidDate *pDate)
{
	CString  szLog="CBaseNetAgreement::DoChangeValidDate()  begin";
	__SendBaseNetLog(szLog);
	if(!pDate)
		return;
    CChangeValidDate  ValidDate;
	memcpy(&ValidDate,pDate,sizeof(CChangeValidDate));
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_ERROR;
	MIANMSGTYPE  mainType=MAINMSG_CHANGEUSERVALIDATE;
	CString szUserID=pDate->szUserId;
	CString szOldValiDate=pDate->szOldValidDate;
	CString  szNewValiDate=pDate->szNewValidDate;
	CString  szTwoValiDate=pDate->sztwoValidDate;
	szUserID.TrimLeft(),szUserID.TrimRight();
	szOldValiDate.TrimLeft(),szOldValiDate.TrimRight();
	szNewValiDate.TrimLeft(),szNewValiDate.TrimRight();
	szTwoValiDate.TrimLeft(),szTwoValiDate.TrimRight();
	if(szNewValiDate!=szTwoValiDate ||szUserID.GetLength()>29 ||szOldValiDate.GetLength()>29 ||szNewValiDate.GetLength()>29 )
	{
		SUBType=SUBMSG_REQUES_ERROR;
	}
	else
	{//参数正确的时候;
		if(g_DBWork.IsUserValiDate(szUserID,szOldValiDate)==FALSE)
			SUBType=SUBMSG_REQUES_ERROR;
		else
		{
			//如果提交的旧密码是正确的就修改密码;
			if(g_DBWork.UpdateUserValiDate(szUserID,szNewValiDate)<0)
			{
				SUBType=SUBMSG_REQUES_ERROR;
				
			}
			else
				SUBType=SUBMSG_REQUES_SUCESSFUL;
		}
	}
	
	
	SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&ValidDate,sizeof(ValidDate));			
	
	
	szLog="CBaseNetAgreement::DoChangeValidDate()  end";
    __SendBaseNetLog(szLog);
	return ;

}

//DEL int CBaseNetAgreement::UpdateUserValiDate(CString szUserId, CString szValiDate)
//DEL {
//DEL 
//DEL }

void CBaseNetAgreement::DoUserQueryEquimentInfo(CUserQueryEquimentInfo *pDate)
{
	CString  szLog="CBaseNetAgreement::DoUserQueryEquimentInfo()  begin";
	__SendBaseNetLog(szLog);
	if(!pDate)
		return;
    CUserQueryEquimentInfo  info;
	memcpy(&info,pDate,sizeof(CUserQueryEquimentInfo));
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_ERROR;
	MIANMSGTYPE  mainType=MAINMSG_CLIENTQUERY;
	CString szUserID=pDate->szUserId;
	CString szEuqimentID=pDate->szEuqimentId;
	CString  szValiDate=pDate->szUserValidDate;

	szUserID.TrimLeft(),szUserID.TrimRight();
	szEuqimentID.TrimLeft(),szEuqimentID.TrimRight();
	szValiDate.TrimLeft(),szValiDate.TrimRight();

	if(szEuqimentID.GetLength()>15 ||szUserID.GetLength()>29 ||szValiDate.GetLength()>29 )
	{
		SUBType=SUBMSG_REQUES_ERROR;
	}
	else
	{//参数正确的时候;
		if(g_DBWork.IsUserValiDate(szUserID,szValiDate)==FALSE)
			SUBType=SUBMSG_REQUES_ERROR;
		else
		{
			//如果验证号正确，就验证这个设备是不是该用户的
			if(g_DBWork.IsUserSEquiment(szUserID,szEuqimentID)<0)
			{
				SUBType=SUBMSG_REQUES_ERROR;
				
			}
			else
				SUBType=SUBMSG_REQUES_SUCESSFUL;
		}
	}
	
	
	SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info,sizeof(info));			
	
	
	szLog="CBaseNetAgreement::DoUserQueryEquimentInfo()  end";
    __SendBaseNetLog(szLog);
	return ;

}

//DEL BOOL CBaseNetAgreement::IsUserSEquiment(const CString &szUserId, const CString &szEquimentId)
//DEL {
//DEL 
//DEL }

void CBaseNetAgreement::DoUserPowerInfo(CString szUserId)
{
	CString  szLog="CBaseNetAgreement::DoUserPowerInfo()  begin";
	__SendBaseNetLog(szLog);
	szUserId.TrimLeft(),szUserId.TrimRight();
	if(szUserId.IsEmpty())
	{
		return;
	}
    Userpower  info;
	char  c='0';
	sprintf(info.szUserId,"%s",szUserId);
//	memcpy((info.szPower),(int)c,250);
	info.beUsed=TRUE;	
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_SUCESSFUL;
	MIANMSGTYPE  mainType=MAINMSG_USERPOWER;//;

	
	if(g_DBWork.GetUserPower(szUserId,info.szPower,300)<0)
	{

		SUBType=SUBMSG_REQUES_ERROR;
	}
	else
	{//参数正确的时候;
		
		SUBType=SUBMSG_REQUES_SUCESSFUL;
	
	}	
	SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info,sizeof(info));			
	
	
	szLog="CBaseNetAgreement::DoUserPowerInfo()  end";
    __SendBaseNetLog(szLog);
	return ;

}

void CBaseNetAgreement::DoSoftVersionInfo()
{
	CString  szLog="CBaseNetAgreement::DoSoftVersionInfo()  begin";
	__SendBaseNetLog(szLog);

    SoftVersionInfo  info;
	CString  szName="MT001";
	sprintf(info.szName,"%s",szName);
//	info.=1;
	
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_SUCESSFUL;
	MIANMSGTYPE  mainType=MAINMSG_SOFTVERSION;//;
	
	//

	CString  szComPanyName;
	int  iVersion=0;
	if(g_DBWork.GetSoVersion("",szComPanyName,iVersion)<0)
	{
		
		SUBType=SUBMSG_REQUES_ERROR;
	}
	else
	{
		info.iVersion=iVersion;
		sprintf(info.szName,"%s",szComPanyName);
	}
	SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info,sizeof(info));			
	
	
	szLog="CBaseNetAgreement::DoSoftVersionInfo()  end";
    __SendBaseNetLog(szLog);
	return ;

}

void CBaseNetAgreement::DoMessageToClient(CMessageToClient *pDate)
{
	CString  szLog="CBaseNetAgreement::DoMessageToClient()  begin";
	__SendBaseNetLog(szLog);
	if(!pDate)
		return;
    CMessageToClient  info;

	SUBMSGTYPE  SUBType=SUBMSG_REQUES_ERROR;
	MIANMSGTYPE  mainType=MAINMSG_INFORMCLIENT;

	{//参数正确的时候;
		if(g_DBWork.InsertTelMessage(pDate)==FALSE)
			SUBType=SUBMSG_REQUES_ERROR;
		else
		{
	
		
			SUBType=SUBMSG_REQUES_SUCESSFUL;
		}
	}
	
	
	SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info,sizeof(info));			
	
	
	szLog="CBaseNetAgreement::DoMessageToClient()  end";
    __SendBaseNetLog(szLog);
	return ;

}

void CBaseNetAgreement::DoOpeateResult(COpeateResult *pDate)
{
	CString  szLog="CBaseNetAgreement::DoOpeateResult()  begin";
	__SendBaseNetLog(szLog);
	if(!pDate)
		return;
    COpeateResult  info;
	
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_ERROR;
	MIANMSGTYPE  mainType=MAINMSG_OPATERRESULT;
	CString szUserID=pDate->USERID;

    CString szTime=pDate->TIME;

	CString  szResult=pDate->RESULT;
	CString  szinfo=pDate->ALARMINFO;

	szUserID.TrimLeft(),szUserID.TrimRight();
	szTime.TrimLeft(),szTime.TrimRight();
	szResult.TrimLeft(),szResult.TrimRight();
	szinfo.TrimLeft(),szinfo.TrimRight();
//	if(szNewValiDate!=szTwoValiDate ||szUserID.GetLength()>29 ||szOldValiDate.GetLength()>29 ||szNewValiDate.GetLength()>29 )
//	{
//		SUBType=SUBMSG_REQUES_ERROR;
//	}
//	else
	{//参数正确的时候;
		if(g_DBWork.InsertOpaterResult(pDate)==FALSE)
			SUBType=SUBMSG_REQUES_ERROR;
		else
		{

			SUBType=SUBMSG_REQUES_SUCESSFUL;
		}
	}
	
	
	SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info,sizeof(info));			
	
	
	szLog="CBaseNetAgreement::DoOpeateResult()  end";
    __SendBaseNetLog(szLog);
	return ;
	

}

int CBaseNetAgreement::SendTelMessage(CMessageToClient *pDate)
{

	return 0;
}

void CBaseNetAgreement::DoCtrTels(CCtrTels *pDate)
{
	CString  szLog="CBaseNetAgreement::DoCtrTels()  begin";
	__SendBaseNetLog(szLog);
	if(!pDate)
		return;
    CCtrTels  info;
	
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_ERROR;
	MIANMSGTYPE  mainType=MAINMSG_CTRTELDATEDOWN;
	CString szEuiqmentID=pDate->EUQIMENTID;  
	
	szEuiqmentID.TrimLeft(),szEuiqmentID.TrimRight();
    if(szEuiqmentID.IsEmpty())
	{
		SUBType=SUBMSG_REQUES_ERROR;
	}
	else
	{//参数正确的时候;
		if(g_DBWork.GetCtrTels(szEuiqmentID,info)<0)
			SUBType=SUBMSG_REQUES_ERROR;
		else
		{
			
			SUBType=SUBMSG_REQUES_SUCESSFUL;
		}
	}	
	SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info,sizeof(info));	
	szLog="CBaseNetAgreement::DoCtrTels()  end";
    __SendBaseNetLog(szLog);
	return ;

}

void CBaseNetAgreement::DoDownLaodAlarmRecvTels(CAlarmRecvTels *pDate)
{
	CString  szLog="CBaseNetAgreement::DoDownLaodAlarmRecvTels()  begin";
	__SendBaseNetLog(szLog);
	if(!pDate)
		return;
    CAlarmRecvTels  info;
	
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_ERROR;
	MIANMSGTYPE  mainType=MAINMSG_SENDTELDATEDOWN;
	CString szEuiqmentID=pDate->EQUIMENTID;  
	
	szEuiqmentID.TrimLeft(),szEuiqmentID.TrimRight();
    if(szEuiqmentID.IsEmpty())
	{
		SUBType=SUBMSG_REQUES_ERROR;
	}
	else
	{//参数正确的时候;
		if(g_DBWork.GetAlarmRecvTels(szEuiqmentID,info)<0)
			SUBType=SUBMSG_REQUES_ERROR;
		else
		{
			
			SUBType=SUBMSG_REQUES_SUCESSFUL;
		}
	}	
	SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info,sizeof(info));	
	szLog="CBaseNetAgreement::DoDownLaodAlarmRecvTels()  end";
    __SendBaseNetLog(szLog);
	return ;
	

}

void CBaseNetAgreement::DoDownLaodFeeInfo(CFeeInfo *PDate)
{
	CString  szLog="CBaseNetAgreement::DoDownLaodFeeInfo()  begin";
	__SendBaseNetLog(szLog);
	if(!PDate)
		return;
    CFeeInfo  info;
	
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_ERROR;
	MIANMSGTYPE  mainType=MAINMSG_FEEINFODATEDOWN;
	CString szEuiqmentID=PDate->EQUIMENTID;  
	
	szEuiqmentID.TrimLeft(),szEuiqmentID.TrimRight();
    if(szEuiqmentID.IsEmpty())
	{
		SUBType=SUBMSG_REQUES_ERROR;
	}
	else
	{//参数正确的时候;
		if(g_DBWork.GetFeeInfo(szEuiqmentID,info)<0)
			SUBType=SUBMSG_REQUES_ERROR;
		else
		{
			
			SUBType=SUBMSG_REQUES_SUCESSFUL;
		}
	}	
	SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info,sizeof(info));	
	szLog="CBaseNetAgreement::DoDownLaodFeeInfo()  end";
    __SendBaseNetLog(szLog);
	return ;

}
void CBaseNetAgreement::DoDownLaodGpsAlarmInfo(CGpsAlarmInfo *pDate)
{
	CString  szLog="CBaseNetAgreement::DoDownLaodGpsAlarmInfo()  begin";
	__SendBaseNetLog(szLog);
	if(!pDate)
		return;
 
	
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_ERROR;
	MIANMSGTYPE  mainType=MAINMSG_ALARMINFODATEDOWN;
	CString szEuiqmentID=pDate->EuqimentId;
	
	szEuiqmentID.TrimLeft(),szEuiqmentID.TrimRight();
    if(szEuiqmentID.IsEmpty())
	{
		SUBType=SUBMSG_REQUES_ERROR;
	}
	else
	{//参数正确的时候;
         TEMP_GpsAlarmInfo  ainfo;
		if(g_DBWork.GetGpsAlarmInfo(szEuiqmentID,ainfo)<0)
			SUBType=SUBMSG_REQUES_ERROR;
		else
		{
			int  iCount  =ainfo.GetSize();
			SUBType=SUBMSG_REQUES_SUCESSFUL;
			for(int iLoop=0;iLoop<iCount;iLoop++)
			{
				CGpsAlarmInfo  info=ainfo.GetAt(iLoop);
				SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info,sizeof(info));				
			}
			SUBType=SUBMSG_DATADOWNLOAD_FINISHED;  //数据下在结束;
			CGpsAlarmInfo  info;
			sprintf(info.EuqimentId,"%s",szEuiqmentID);
			SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info,sizeof(info));				
					
		}
	}	
	szLog="CBaseNetAgreement::DoDownLaodGpsAlarmInfo()  end";
    __SendBaseNetLog(szLog);
	return ;

}
//更新报警接收手机
void CBaseNetAgreement::DoUpdateAlarmRecvTels(CAlarmRecvTels *pDate)
{
	CString  szLog="CBaseNetAgreement::DoUpdateAlarmRecvTels()  begin";
	__SendBaseNetLog(szLog);
	if(!pDate)
		return;
    CAlarmRecvTels  info;
	memcpy(&info,pDate,sizeof(CAlarmRecvTels));
	
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_ERROR;
	MIANMSGTYPE  mainType=MAINMSG_UPDATESENDTEL;
	CString szEuiqmentID=pDate->EQUIMENTID;
	CString  szTels=pDate->CONTACTTEL;
	
	szEuiqmentID.TrimLeft(),szEuiqmentID.TrimRight();
	szTels.TrimLeft(),szTels.TrimRight();
    if(szEuiqmentID.IsEmpty() ||szTels.IsEmpty())
	{
		SUBType=SUBMSG_REQUES_ERROR;
	}
	else
	{//参数正确的时候;
		if(g_DBWork.UpdateAlarmRecvTels(szEuiqmentID,szTels)<0)
			SUBType=SUBMSG_REQUES_ERROR;
		else
		{
			
			SUBType=SUBMSG_REQUES_SUCESSFUL;
		}
	}	
	SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info,sizeof(info));	
	szLog="CBaseNetAgreement::DoUpdateAlarmRecvTels()  end";
    __SendBaseNetLog(szLog);
	return ;	
}


//更新控制手机信息;
void CBaseNetAgreement::DoUpDateCtrTels(CCtrTels *pDate)
{
	CString  szLog="CBaseNetAgreement::DoUpDateCtrTels()  begin";
	__SendBaseNetLog(szLog);
	if(!pDate)
		return;
    CCtrTels  info;
	memcpy(&info,pDate,sizeof(info));
	
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_ERROR;
	MIANMSGTYPE  mainType=MAINMSG_UPDATECTRTEL;
	CString szEuiqmentID=pDate->EUQIMENTID;  
	CString  szTels=pDate->TELS;
	
	szEuiqmentID.TrimLeft(),szEuiqmentID.TrimRight();
	szTels.TrimLeft(),szTels.TrimRight();
    if(szEuiqmentID.IsEmpty() ||  szTels.IsEmpty())
	{
		SUBType=SUBMSG_REQUES_ERROR;
	}
	else
	{//参数正确的时候;
		if(g_DBWork.UpdateCtrTels(szEuiqmentID,szTels)<0)
			SUBType=SUBMSG_REQUES_ERROR;
		else
		{
			
			SUBType=SUBMSG_REQUES_SUCESSFUL;
		}
	}	
	SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info,sizeof(info));	
	szLog="CBaseNetAgreement::DoUpDateCtrTels()  end";
    __SendBaseNetLog(szLog);
	return ;
	
}
//更新报警信息
void CBaseNetAgreement::DoUpDatepsAlarmInfo(CGpsAlarmInfo *pDate)
{
	CString  szLog="CBaseNetAgreement::DoUpDatepsAlarmInfo()  begin";
	__SendBaseNetLog(szLog);
	if(!pDate)
		return;	
	CGpsAlarmInfo  info;
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_ERROR;
	MIANMSGTYPE  mainType=MAINMSG_UPDATEALARMINFO;
	CString szEuiqmentID=pDate->EuqimentId;
	
	szEuiqmentID.TrimLeft(),szEuiqmentID.TrimRight();
    if(szEuiqmentID.IsEmpty())
	{
		SUBType=SUBMSG_REQUES_ERROR;
	}
	else
	{//参数正确的时候;
		if(g_DBWork.UpDateGpsAlarmInfo(pDate)<0)
			SUBType=SUBMSG_REQUES_ERROR;
		else
		{
			SUBType=SUBMSG_REQUES_SUCESSFUL;		
		}
		SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info,sizeof(info));				
		
	}	
	szLog="CBaseNetAgreement::DoUpDatepsAlarmInfo()  end";
    __SendBaseNetLog(szLog);
	return ;
	
}

//下载用户可用地图列表;
void CBaseNetAgreement::DoDownLoadUserMapList(CUserMapInfo *pDate)
{

	CString  szLog="CBaseNetAgreement::DoDownLoadUserMapList()  begin";
	__SendBaseNetLog(szLog);
	if(!pDate)
		return;	
	
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_ERROR;
	MIANMSGTYPE  mainType=MAINMSG_DOWNLOADMAP;

	CString  szUserId=pDate->USERID;
	
	szUserId.TrimLeft(),szUserId.TrimRight();
    if(szUserId.IsEmpty())
	{
		SUBType=SUBMSG_REQUES_ERROR;
	}
	else
	{//参数正确的时候;
		CStringArray asUserMap;
		if(g_DBWork.GetUserMap(szUserId,asUserMap)<0)
			SUBType=SUBMSG_REQUES_ERROR;
		else
		{
			int  iCount=asUserMap.GetSize();
			for(int iloop=0;iloop<iCount;iloop++)
			{
				CString  filename=asUserMap.GetAt(iloop);
				CUserMapInfo  info;
				sprintf(info.USERID,"%s",szUserId);
				sprintf(info.MAPNAME,"%s",filename);
				SUBType=SUBMSG_REQUES_SUCESSFUL;
				SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info,sizeof(info));
				
			}
			CUserMapInfo  info1;
			sprintf(info1.USERID,"%s",szUserId);
            SUBType=SUBMSG_DATADOWNLOAD_FINISHED;
			SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info1,sizeof(info1));
					
		}
						
		
	}	
	szLog="CBaseNetAgreement::DoDownLoadUserMapList()  end";
    __SendBaseNetLog(szLog);
	return ;

}

//心跳测试;addede  by  20070325
void CBaseNetAgreement::DoHeartBeat()
{
	CString  szLog="CBaseNetAgreement::DoHeartBeat()  begin";
	__SendBaseNetLog(szLog);	
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_SUCESSFUL;
	MIANMSGTYPE  mainType=MAINMSG_HEARTBEAT;
	//int  fd=this->m_fd;
    //得到登陆用户的信息;
	
	g_ClientManagerLock.Lock();
	ClintNode *pNode=theApp.m_Clientmanager.GetClintNode(this->m_fd); 
	if(pNode==NULL)
	{
	}
	else
		pNode->iRecvHeartbeatCount=0;
	g_ClientManagerLock.Unlock();
	
	CResponCommandInfo  info;
	SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&info,sizeof(info));	
	szLog="CBaseNetAgreement::DoHeartBeat()  end";
    __SendBaseNetLog(szLog);
	return ;
	
}
//查询gps数据
void CBaseNetAgreement::DoGpsDateQuery(CGpsDataQuery *pDate)
{
	CString  szLog="CBaseNetAgreement::DoGpsDateQuery()  begin";
	__SendBaseNetLog(szLog);
	if(!pDate)
		return;
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_ERROR;
	MIANMSGTYPE  mainType=MAINMSG_GPSDATAQUERY;
	
	CString  szEquimentId=pDate->szEquimentID;
	szEquimentId.TrimLeft(),szEquimentId.TrimRight();
    if(szEquimentId.IsEmpty())
	{
		SUBType=SUBMSG_REQUES_ERROR;
	}
	else
	{//参数正确的时候;	
	//	else
		{
			CGPSData data;
			if(g_DBWork.GetGpsNewestTime(szEquimentId,data)<0)
			{

				SUBType=SUBMSG_REQUES_ERROR;
				
			}
			else
			{
				SendToNetMessage(MSGTYPE_GPS,MAINMSG_GPSDATATOCLIENT,SUBMSG_REQUES_SUCESSFUL,(char *)&data,sizeof(data));
				szLog="CBaseNetAgreement::DoGpsDateQuery()  end";
				__SendBaseNetLog(szLog);
				return ;
			}	

		}
		
		
	}
    CGpsDataQuery  querydate;
	SendToNetMessage(MSGTYPE_COMMON,mainType,SUBType,(char *)&querydate,sizeof(querydate));
				
	szLog="CBaseNetAgreement::DoGpsDateQuery()  end";
    __SendBaseNetLog(szLog);
	return ;

}

//查询用户的最新gps数据;
void CBaseNetAgreement::DoQueryUserGpsData(CQueryNewestGpsData *pDate)
{
	CString szLog = "CBaseNetAgreement::DoGpsDateQuery()  begin";
	__SendBaseNetLog(szLog);

	if(!pDate)
	{
		return;
	}

	SUBMSGTYPE SUBType = SUBMSG_REQUES_ERROR;
	MIANMSGTYPE mainType = MAINMSG_GPSDATAQUERY;
	
	CString szUserId = pDate->szUsrID;
	szUserId.TrimLeft(), szUserId.TrimRight();
    if(szUserId.IsEmpty())
	{
		SUBType = SUBMSG_REQUES_ERROR;
	}
	else
	{
        CObArray asGpsDate;
		if(g_DBWork.GetUserGpsNewestTime(szUserId, asGpsDate) < 0)
		{
			SUBType = SUBMSG_REQUES_ERROR;
		}
		else
		{
			int iCount = asGpsDate.GetSize();
			for(int iLoop=0; iLoop < iCount; iLoop ++)
			{
				CGPSData* pGpsDate = (CGPSData*)asGpsDate.GetAt(iLoop);
				if(pGpsDate)
				{
					SendToNetMessage(MSGTYPE_GPS, MAINMSG_GPSDATATOCLIENT, SUBMSG_REQUES_SUCESSFUL, (char *)pGpsDate, sizeof(CGPSData));
				}

				Sleep(100);
			}

			for(int index = 0; index < iCount; index ++)
			{
				CGPSData* pGpsDate = (CGPSData*)asGpsDate.GetAt(index);
				if(pGpsDate)
				{
					delete pGpsDate;
					pGpsDate = NULL;
				}
			}
		}	
	}	

	szLog = "CBaseNetAgreement::DoGpsDateQuery()  end";
    __SendBaseNetLog(szLog);
	
	return;
}

void CBaseNetAgreement::DoQueryGoogleMap(QueryGoogleMap *pDate)
{
	CString  szLog="CBaseNetAgreement::DoQueryGoogleMap()  begin";
	__SendBaseNetLog(szLog);
	if(!pDate)
		return;
	SUBMSGTYPE  SUBType=SUBMSG_REQUES_ERROR;
	MIANMSGTYPE  mainType=MAINMSG_GOOGLEMAPQUERY;
	
	CString  szUserId=pDate->UserId;
	CString  szEquimentID=pDate->EquimentID;
	szUserId.TrimLeft(),szUserId.TrimRight();
	szEquimentID.TrimLeft(),szEquimentID.TrimRight();
    if(szUserId.IsEmpty() || szEquimentID.IsEmpty())
	{
		SUBType=SUBMSG_REQUES_ERROR;
	}
	else
	{//参数正确的时候;	
		//	else
		{
			//GetGpsNewestTime(CString szEquientId, CGPSData &data)
			CGPSData  data;
			if(g_DBWork.GetGpsNewestTime(szEquimentID,data)<0)
			{
				
				SUBType=SUBMSG_REQUES_ERROR;
				
			}
			else
			{
				SendToNetMessage(MSGTYPE_COMMON,MAINMSG_GOOGLEMAPQUERY,SUBType,(char *)pDate,sizeof(QueryGoogleMap));
				
			
			}
		}
	}				
	szLog="CBaseNetAgreement::DoQueryGoogleMap()  end";
    __SendBaseNetLog(szLog);
	//	AfxMessageBox("3");
	
	return ;
	
}


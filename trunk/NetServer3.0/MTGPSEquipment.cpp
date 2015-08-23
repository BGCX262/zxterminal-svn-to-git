// MTGPSEquipment.cpp: implementation of the CMTGPSEquipment class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MTGPSEquipment.h"
#include "GmemoryDataManager.h"
#include "DBWork.h"
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
extern CGmemoryDataManager  g_GmemoryDataManager;
extern  CDBWork   g_DBWork;
extern  CMutex               g_GpsAlarmInfoLock;

void   __SendGpsLog(char  * buf)
{
	ofstream  f("SendGpsLog.Log",ios::app| ios::out);
	f<<buf<<endl;
	f.close();
}
#define   MINLENTCP    19  
#define   MINLENUDP    20
#define   MTGPSMIMLEN    80  //MT GPS的最小数据长度
#define   MAXCOMMANDID  30
//V1
typedef  struct  tagMTGPSV1Data 
{

}MTGPSV1Data;


//V4  2
typedef  struct  tagMTGPSV42Data 
{

}MTGPSV42Data;

//V4  3
typedef  struct  tagMTGPSV43Data 
{

}MTGPSV43Data;


const int CMTGPSEquipment::iDataNeedLen[5]={0,80,80,26,20};

CMTGPSEquipment::CMTGPSEquipment()
{

}

CMTGPSEquipment::~CMTGPSEquipment()
{

}

//检测数据的合法性;
BOOL   CMTGPSEquipment::IsDataLegalAndFindPackNum(char *p,int  iLen,int& INumber,int &iEnd)
{
	if(!p)
		return  FALSE;	
	//验证的时候要加入数据类型来验证类型
	if(iLen<MTGPSMIMLEN)
		return  FALSE;
	if(p[0]!='*')
		return  FALSE;
//	if(p[iLen-2]!='#')
//		return  FALSE;

	MTGPSDATATYPE  msgtype=this->GetDataType(p,iLen);
	int ineedLen=iDataNeedLen[msgtype];
	if(iLen<ineedLen   ||  ineedLen==0)
		return  FALSE;	
	return  TRUE;
}
BOOL   CMTGPSEquipment::IsRegiste(CIOCPBuff  *pIOBuf)
{
	if(!pIOBuf)
		return FALSE;
	BOOL  bReg=FALSE;
	char *p=pIOBuf->GetWSABuffer()->buf;
	if(!p)
		return FALSE;
	if(p[13]=='6' && p[14]=='6'&& p[15]=='6'&& p[16]=='0')
	{//注册信息;
		bReg=TRUE;
		return  bReg;
	}
	if(p[13]=='6' && p[14]=='6'&& p[15]=='6'&& p[16]=='6')
	{//注册信息;
		bReg=TRUE;
		return  bReg;
	}
	if(p[13]=='6' && p[14]=='0'&& p[15]=='6'&& p[16]=='0')
	{//注册信息;
		bReg=TRUE;
		return  bReg;
	}

	return  bReg;
}

//是不是要回复的
BOOL   CMTGPSEquipment::IsReGpsMessage(CIOCPBuff  *pIOBuf)
{
	if(!pIOBuf)
		return FALSE;
	BOOL  bReg=FALSE;
	char *p=pIOBuf->GetWSABuffer()->buf;
	if(!p)
		return FALSE;
	if(p[13]=='6' && p[14]=='6'&& p[15]=='6'&& p[16]=='0')
	{//注册信息;
		bReg=TRUE;
		return  bReg;
	}
	if(p[13]=='6' && p[14]=='6'&& p[15]=='6'&& p[16]=='6')
	{//注册信息;
		bReg=TRUE;
		return  bReg;
	}	
	return  bReg;
}


BOOL CMTGPSEquipment::ParseGpsData(CIOCPBuff *pIOBuf,CGPSData  &GpsData,CString &szMessage)
{
	if(!pIOBuf)
		return FALSE;
	int iNumber=0,iEnd=pIOBuf->iMsgLen;
    if(!IsDataLegalAndFindPackNum(pIOBuf->buf,pIOBuf->iMsgLen,iNumber,iEnd))
		return  FALSE;
	if(IsRegiste(pIOBuf))
	{//进行设备的注册；主要是记录设备的连接信息；

		CString   szKey;		 
		char  *p=pIOBuf->GetWSABuffer()->buf;
		char   temp[15]={0};
		memcpy(temp,&p[1],11);
		szKey=temp;
		this->m_EquimenList.UpdatUser(pIOBuf->fd,szKey,pIOBuf->addr);
		if(IsReGpsMessage(pIOBuf))
		{
			ReGPSMessage(pIOBuf->fd,pIOBuf->addr,"W");
		}
		
	}
	return  DecGpsData(pIOBuf,GpsData,szMessage);
}

BOOL CMTGPSEquipment::DecGpsData(CIOCPBuff *p, CGPSData &GpsData,CString &szMessage)
{
	char  *pData=p->GetWSABuffer()->buf;  
	if(!pData)
		return  FALSE;

	return TRUE;

}



UINT CMTGPSEquipment::SetState(char *pState)
{
	return  0;

}
MTGPSDATATYPE CMTGPSEquipment::GetDataType(char *p, int iLen)
{
	if(!p)
		return  TYPE_MTGPSNULL;
	MTGPSDATATYPE  MTGPStype=TYPE_MTGPSNULL;

   if(p[13]=='6' && p[14]=='0'&& p[15]=='6'&& p[16]=='0')
   {//注册信息;
	   return 	 TYPE_MTGPSREG;
   }
   else
	   return TYPE_MTGPSGPS;
	return  MTGPStype;
	
}


//处理所有的命令的函数;
BOOL CMTGPSEquipment::SendMTGPSCommand(CString szEquimentKey, const CGpsCommand *pCommand)
{
	CString  szLog;
	szLog.Format("CMTGPSEquipment::SendMTGPSCommand begin  %s  ",szEquimentKey);
	char  logbuf[1000]={0}; 
	sprintf(logbuf,"%s",szLog);
	__SendGpsLog(logbuf);

	if(!pCommand)
		return FALSE;
	EquiNetInfo *pNetInfo=this->m_EquimenList.Find(szEquimentKey);
	//发送命令
	if(!pNetInfo)
	{
		szLog="CMTGPSEquipment::SendMTGPSCommand pNetInfo  ==NULL  return  FALSE ";
		memset( logbuf,0,1000);
		sprintf(logbuf,"%s",szLog);
		__SendGpsLog(logbuf);
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
		__SendGpsLog(logbuf);
		return  FALSE;
	}
	char bufcmd[1024];
	if(!PackCommand(pCommand,bufcmd,1024))
	{
		szLog="CMTGPSEquipment::SendMTGPSCommand 不能解析控制命令  return  FALSE ";
		memset( logbuf,0,1000);
		sprintf(logbuf,"%s",szLog);
		__SendGpsLog(logbuf);
		return  FALSE;
	}
	//  CString  szTemp="*13370270190,000000,S01010111#ff";
	//  sprintf(bufcmd,"%s",szTemp);
	// int i=m_SendSock.SendTo(bufcmd,strlen(bufcmd),(SOCKADDR *)&addr,sizeof(pNetInfo->addr),0);
	
	// Sendfd
	int i=sendto(fd,bufcmd,strlen(bufcmd),0,(SOCKADDR *)&addr,sizeof(pNetInfo->addr));
	if(i<=0)
	{
		szLog="i<=0  return  FALSE ";
		memset( logbuf,0,1000);
		sprintf(logbuf,"%s",szLog);
		__SendGpsLog(logbuf);
		
		return FALSE;
	}
	int irroe=WSAGetLastError();
	//en
	__SendGpsLog(bufcmd);
	return  TRUE;
}

BOOL CMTGPSEquipment::PackCommand(const CGpsCommand *pCommand,char buf[],int iLen)
{
	//addede  20070423;
	
	CString  szCommandinfo[MAXCOMMANDID]={"0010","0020","0030","0040","0050",
		"0060","5010","0230","0080","0130","0070","1060","1070",
		"0110","0120","1010","1020","0200","0190","0000","0170","0180",
		"0140","0150","0160","0100","0220","0210","0240","0250"};
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
	if(iCommandID<0  ||iCommandID>=MAXCOMMANDID)
	{
		return  FALSE;
	}
	//*FFFFFFFFFFF,PPPPPP,XXXX,ZZ…ZZ#CC
	CString  szCommandText=szCommandinfo[iCommandID];
	if(szCommandText.IsEmpty())
		return  FALSE;
	CString  szPWS=pCommand->PWS;
	if(szPWS.IsEmpty())
		return  FALSE;
	CString  szCommand;
	szCommand.Format("*%s,%s,%s#FF",szEquimentKey,szPWS,szCommandText);
	memset(buf,0,iLen);
	sprintf(buf,"%s",szCommand);
	return  TRUE;	

}


BOOL CMTGPSEquipment::SendSMSAndWebCommand(CString szEquimentKey, const CString &szCommand,BOOL  bWEB)
{
	if(bWEB)
	{//web
		return   SendCharCommand(szEquimentKey,szCommand);
		
	}
	CString  szCommandText[MAXCOMMANDID]={"设定防盗","静音设防","部分设防","防抢","防抢解除",
		"解除防盗","求救收到","启动报警","开车尾箱","当前状态","查询车辆","实时监控","停止监控",
		"启动遥控","屏蔽遥控","开监听","关监听","学习遥控","消除遥控","设置","开对话","关对话",
		"灵敏度一","灵敏度二","灵敏度三","断油熄火","熄火恢复","紧急解除","锁车","解除锁车"};

	BOOL   bFind=FALSE;
	int     index=-1;
	for(int  iLoop=0;iLoop<MAXCOMMANDID;iLoop++)
	{
		if(szCommand==szCommandText[iLoop])
		{
			bFind=TRUE;
			index=iLoop;
			break;
		}
	}
	if(!bFind)
		return  FALSE;
   CGpsCommand   commandinfo;
   sprintf(commandinfo.EquimentId,"%s",szEquimentKey);
   commandinfo.iCOmmandID=index;
   CString szPws="000000";
   sprintf(commandinfo.PWS,"%s",szPws);
   return   SendMTGPSCommand(szEquimentKey,&commandinfo);
	
}

//处理所有的命令的函数;
BOOL CMTGPSEquipment::SendCharCommand(CString szEquimentKey,  const CString &szCommand)
{
	EquiNetInfo *pNetInfo=this->m_EquimenList.Find(szEquimentKey);
	//发送命令
	if(!pNetInfo)
		return FALSE;
	
	struct  sockaddr_in   addr=pNetInfo->addr;
	// addr.sin_port=htons(addr.sin_port);
	int   fd=pNetInfo->fd;
	if(fd==INVALID_SOCKET)
		return  FALSE;
	char bufcmd[1024];
 
	if (szCommand.IsEmpty())
	{
		return  FALSE;
	}
	else
	{
		sprintf(bufcmd,"%s",szCommand);
	}	
	// Sendfd
	int i=sendto(fd,bufcmd,strlen(bufcmd),0,(SOCKADDR *)&addr,sizeof(pNetInfo->addr));
	if(i<=0)
	{		
		return FALSE;
	}
	int irroe=WSAGetLastError();
	//en
	__SendGpsLog(bufcmd);
	return  TRUE;
}

//回复车机
BOOL CMTGPSEquipment::ReGPSMessage(int fd,struct sockaddr_in addr, const CString &szMsg)
{
	if(fd==INVALID_SOCKET)
		return  FALSE;
	char bufcmd[1024];
	CString  szTemp;
	if(szMsg.IsEmpty())
	{
		szTemp="W";
	}
	else
		szTemp=szMsg;
	sprintf(bufcmd,"%s",szTemp);
	int i=sendto(fd,bufcmd,strlen(bufcmd),0,(SOCKADDR *)&addr,sizeof(addr));
	if(i<=0)
	{
		
		return FALSE;
	}
	int irroe=WSAGetLastError();
	//en
	__SendGpsLog(bufcmd);
	return  TRUE;
}


void CMTGPSEquipment::GetCtrTel(CString szCtrTel, CStringArray &aTel)
{
	//13370270193;13370270191;13370270192;
	aTel.RemoveAll();
	aTel.SetSize(0);
	szCtrTel.TrimLeft(),szCtrTel.TrimRight();
	if(szCtrTel.IsEmpty())
		return;
	while(szCtrTel.GetLength()>11)
	{
		//解析;
		int  index=szCtrTel.Find(";");
		if(index==-1)
			break;
		CString  szSimid=szCtrTel.Mid(0,index);
		aTel.Add(szSimid);
		szCtrTel=szCtrTel.Mid(index+1);
	}
}

// MTJJAEquipment.cpp: implementation of the CMTJJAEquipment class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MTJJAEquipment.h"

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
#define    SET_OK  9999
#define    SET_ERR  9996

#define   MTJJAMIMLEN    19  //家家安的最小数据长度

void   __Log(char  * buf)
{
	static  int  iCount=0;
	char  cccount[100]={0};
	sprintf(cccount,"%d",iCount++);
	ofstream  f("MTLog.Log",ios::app| ios::out);
	f<<cccount<<endl;
	f<<buf<<endl;
	f.close();
}

void   __SendLog(char  * buf)
{
	ofstream  f("MTSend.Log",ios::app| ios::out);
	f<<buf<<endl;
	f.close();
}
typedef  struct  tagTYPEADATA  //A类型的信息;
{
	char  Begin;
	char  EquimentID[12];
	char  TYPE;
	char  moduleNumer[2];
	char  ProbeNumer[2];
	char  Data[5];
	char  End;
	char  Reserve[2];
	tagTYPEADATA()
	{
		Begin=0;
		ZeroMemory(EquimentID,12);
		TYPE=0;
		ZeroMemory(moduleNumer,2);
	
		ZeroMemory(Data,5);
		End=0;
		ZeroMemory(Reserve,2);
		ZeroMemory(ProbeNumer,2);
	}

}TYPEADATA,*LPTYPEADATA;

typedef  struct  tagTYPEADATROK    //设置正确
{
	char  Begin;
	char  EquimentID[12];
	char  TYPE[4];
	char  End;
	char  Reserve[2];
	tagTYPEADATROK()
	{
		Begin=0;
		ZeroMemory(EquimentID,12);
		ZeroMemory(TYPE,4);
		End=0;
		ZeroMemory(Reserve,2);
	}	
}TYPEADATROK,*LPTYPEADATROK;


typedef  struct  tagTYPEADATERR  //返回设置错误;
{
	char  Begin;
	char  EquimentID[12];
	char  TYPE[4];
	char  End;
	char  Reserve[2];
	tagTYPEADATERR()
	{
		Begin=0;
		ZeroMemory(EquimentID,12);
		ZeroMemory(TYPE,4);
		End=0;
		ZeroMemory(Reserve,2);
	}	
}TYPEADATERR ,*LPTYPEADATERR;


typedef  struct  tagTYPEADATREG  //注册;
{
	char  Begin;
	char  EquimentID[12];
	char  TYPE[5];
	char  End;
	char  Reserve[2];
	tagTYPEADATREG()
	{
		Begin=0;
		ZeroMemory(EquimentID,12);
		ZeroMemory(TYPE,5);
		End=0;
		ZeroMemory(Reserve,2);
	}	
}TYPEADATREG  ,*LPTYPEADATREG ;



const int CMTJJAEquipment::iDataNeedLen[5]={0,20,21,26,20};

CMTJJAEquipment::CMTJJAEquipment()
{


}

CMTJJAEquipment::~CMTJJAEquipment()
{

}
BOOL   CMTJJAEquipment::IsDataLegalAndFindPackNum(char *p,int  iLen,int& INumber,int &iEnd)
{
	if(!p)
		return  FALSE;
	__Log(p);

	//验证的时候要加入数据类型来验证类型
	if(iLen<MTJJAMIMLEN)
		return  FALSE;
	MTJJADATATYPE  msgtype=this->GetDataType(p,iLen);
	int ineedLen=iDataNeedLen[msgtype];
	if(iLen<ineedLen   ||  ineedLen==0)
		return  FALSE;
	if(p[0]!='*')
		return  FALSE;
	switch(msgtype ) {
	case TYPE_ROK:
		{
			if(p[17]!='#')
				return  FALSE;
			break;
		}
		break;
	case TYPE_REG:
		{
			if(p[18]!='#')
				return  FALSE;
			break;
		}
		break;
	case TYPE_A:
		{
			if(p[23]!='#')
				return  FALSE;
			break;
		}
		break;
	case TYPE_ERR:
		{
			if(p[17]!='#')
				return  FALSE;
			break;
		}
		break;
	default:
		return FALSE;
	}
	


	return  TRUE;
}

BOOL CMTJJAEquipment::IsRegiste(CIOCPBuff  *pIOBuf)
{
	if(!pIOBuf)
		return FALSE;
	BOOL  bReg=FALSE;
	char *p=pIOBuf->GetWSABuffer()->buf;
	if(!p)
		return FALSE;
	if(p[13]=='6' && p[14]=='6'&& p[15]=='6'&& p[16]=='6')
	{//注册信息;
		bReg=TRUE;
		return  bReg;
	}
	if(p[13]=='6' && p[14]=='6'&& p[15]=='6'&& p[16]=='0')
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
BOOL CMTJJAEquipment::ParseGuardData(CIOCPBuff  *pIOBuf,CGuardData  &GuardData)
{
	if(!pIOBuf)
		return  FALSE;
     int iNumber=0,iEnd=pIOBuf->iMsgLen;
	 MTJJADATATYPE iDataType=this->GetDataType(pIOBuf->GetWSABuffer()->buf,pIOBuf->iMsgLen);

     if(!this->IsDataLegalAndFindPackNum(pIOBuf->GetWSABuffer()->buf,pIOBuf->iMsgLen,iNumber,iEnd))
		 return  FALSE;
	 if(this->IsRegiste(pIOBuf))
	 {//是注册信息；进行注册
		 CString   szKey;		 
		 char  *p=pIOBuf->GetWSABuffer()->buf;
		 char   temp[15]={0};
		 memcpy(temp,&p[1],11);
		 szKey=temp;		
		 this->m_EquimenList.UpdatUser(pIOBuf->fd,szKey,pIOBuf->addr);	
		 ReMessage(szKey,"6666");
	 }
	 if( this->DecGuardData(pIOBuf,iDataType,GuardData))
	 {

	 }
	 else
		 return FALSE;
	 return TRUE;



}

BOOL CMTJJAEquipment::DecGuardData(CIOCPBuff *p, MTJJADATATYPE iDataType,CGuardData &GuardData)
{
	char  *pData=p->GetWSABuffer()->buf;
	if(!pData)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if(iDataType==TYPE_MTJJANULL)
	{
		ASSERT(FALSE);
		return  FALSE;
	}
	COleDateTime  now=COleDateTime::GetCurrentTime();
	CString  szTime=now.Format("%Y-%m-%d %H:%M:%S");
	if(iDataType==TYPE_A)
	{//报警的信息;
		LPTYPEADATA  pAdata=(LPTYPEADATA)pData;
		pAdata->EquimentID[11]=0;
		pAdata->Data[6]=0;
		strcpy(GuardData.EquimentId,pAdata->EquimentID);
		char szTemp[3]={0};
		memcpy(szTemp,pAdata->ProbeNumer,2);
		GuardData.ProbeType=atoi(szTemp);
		memset(szTemp,0,3);
		memcpy(szTemp,pAdata->moduleNumer,2);
		GuardData.DataType=atoi(szTemp);
		GuardData.state=atoi(pAdata->Data);		
		sprintf(GuardData.szTime,"%s",szTime);
		GuardData.szTime[szTime.GetLength()]=0;
		return  TRUE;
	}
	if(iDataType==TYPE_ERR)
	{//恢复的信息
		LPTYPEADATERR  pERRdata=(LPTYPEADATERR)pData;
		pERRdata->EquimentID[11]=0;
		strcpy(GuardData.EquimentId,pERRdata->EquimentID);
		GuardData.ProbeType=0;
		GuardData.DataType=0;		
		GuardData.state=SET_ERR;		
		sprintf(GuardData.szTime,"%s",szTime);
		GuardData.szTime[szTime.GetLength()]=0;		
		return  TRUE;
	}
	if(iDataType==TYPE_REG)
	{
		LPTYPEADATREG  pRegdata=(LPTYPEADATREG)pData;
		pRegdata->EquimentID[11]=0;
		strcpy(GuardData.EquimentId,pRegdata->EquimentID);
		GuardData.ProbeType=0;
		GuardData.DataType=0;		
		GuardData.state=0;		
		sprintf(GuardData.szTime,"%s",szTime);
		GuardData.szTime[szTime.GetLength()]=0;		
		return  TRUE;
	}
	if(iDataType==TYPE_ROK)
	{
		LPTYPEADATROK  pROKdata=(LPTYPEADATROK)pData;
		pROKdata->EquimentID[11]=0;
		strcpy(GuardData.EquimentId,pROKdata->EquimentID);
		GuardData.ProbeType=0;
		GuardData.DataType=0;		
		GuardData.state=SET_OK;		
		sprintf(GuardData.szTime,"%s",szTime);
		GuardData.szTime[szTime.GetLength()]=0;		
		return  TRUE;
	}
	return TRUE;
}

MTJJADATATYPE CMTJJAEquipment::GetDataType(char *p, int iLen)
{
	if(!p)
		return  TYPE_MTJJANULL;
	MTJJADATATYPE  MTJJAtype=TYPE_MTJJANULL;
	//开始解析类型;  解析类型的时候可以判断它的数据长度;	
	if(p[13]=='A')
	{//报警;
		MTJJAtype=TYPE_A;
	}
	if(p[13]=='E' &&p[14]=='R'&&p[15]=='R')
	{//回复
		MTJJAtype=TYPE_ERR;
	}
	if(p[13]=='R' &&p[14]=='O'&&p[15]=='K')
	{//rok
		MTJJAtype=TYPE_ROK;
	}
	if(p[13]=='6')
	{//注册
		MTJJAtype=TYPE_REG;
	}	
	return  MTJJAtype;

}

//处理所有的命令的函数;
BOOL CMTJJAEquipment::SendCommand(CString szEquimentKey, const CGuardCommand *pCommand)
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
  if(!PackCommand(pCommand,bufcmd,1024))
	  return  FALSE;
//  CString  szTemp="*13370270190,000000,S01010111#ff";
//  sprintf(bufcmd,"%s",szTemp);
 // int i=m_SendSock.SendTo(bufcmd,strlen(bufcmd),(SOCKADDR *)&addr,sizeof(pNetInfo->addr),0);
 
 // Sendfd
  int i=sendto(fd,bufcmd,strlen(bufcmd),0,(SOCKADDR *)&addr,sizeof(pNetInfo->addr));
  if(i<=0)
  {

	  return FALSE;
  }
  int irroe=WSAGetLastError();
  //en
  __SendLog(bufcmd);
  return  TRUE;
}


BOOL CMTJJAEquipment::PackCommand(const CGuardCommand *pCommand, char bufcommand[],int iBufLen)
{

	if(!pCommand  || !bufcommand  ||  iBufLen<=0)
		return  FALSE;
	memset(bufcommand,0,iBufLen);
	switch(pCommand->iCOmmandID)
	{

	   default  :
		   //处理默认的命令格式;
		   {
			   bufcommand[0]='*';
			   memcpy(bufcommand+1,pCommand->EquimentId,strlen(pCommand->EquimentId));
			   bufcommand[strlen(pCommand->EquimentId)+1]=',';
			   char  pws[8]="000000,";
			   memcpy(&bufcommand[strlen(bufcommand)],pws,strlen(pws));
			   memcpy(&bufcommand[strlen(bufcommand)],pCommand->comandText,strlen(pCommand->comandText));
               
			   
			   bufcommand[strlen(bufcommand)]='#';
			   bufcommand[strlen(bufcommand)]='F';
			   bufcommand[strlen(bufcommand)]='F';
		   }
		   break;
	}
	return  TRUE;
	

	//bufcommand


}

BOOL CMTJJAEquipment::ReMessage(CString szEquimentKey, const CString &szMsg)
{
	EquiNetInfo *pNetInfo=this->m_EquimenList.Find(szEquimentKey);
	//发送命令
	if(!pNetInfo)
		return FALSE;

	struct  sockaddr_in   addr=pNetInfo->addr;
	int   fd=pNetInfo->fd;
	if(fd==INVALID_SOCKET)
		return  FALSE;
	char bufcmd[1024];
	CString  szTemp;
	if(szMsg.IsEmpty())
	{
		szTemp.Format("*%s,00000,6666#ff",szEquimentKey);
	}
	else
	{
		szTemp.Format("*%s,00000,%s#ff",szEquimentKey,szMsg);
	}
	sprintf(bufcmd,"%s",szTemp);
	int i=sendto(fd,bufcmd,strlen(bufcmd),0,(SOCKADDR *)&addr,sizeof(pNetInfo->addr));
	if(i<=0)
	{
		
		return FALSE;
	}
	int irroe=WSAGetLastError();
	//en
	__SendLog(bufcmd);
	return  TRUE;
}

BOOL CMTJJAEquipment::SendSMSAndWebCommand(CString szEquimentKey, const CString &szCommand,BOOL  bweb)
{
	if(bweb)
	{
		//web
		return  SendWebCommand(szEquimentKey,szCommand);
	}
	EquiNetInfo *pNetInfo=this->m_EquimenList.Find(szEquimentKey);
	//发送命令
	if(!pNetInfo)
		return FALSE;
	
	struct  sockaddr_in   addr=pNetInfo->addr;
	int   fd=pNetInfo->fd;
	if(fd==INVALID_SOCKET)
		return  FALSE;
	char bufcmd[1024]={0};
	sprintf(bufcmd,"%s",szCommand);
	int i=sendto(fd,bufcmd,strlen(bufcmd),0,(SOCKADDR *)&addr,sizeof(pNetInfo->addr));
	if(i<=0)
	{
		int irroe=WSAGetLastError();
		__SendLog(bufcmd);
		return FALSE;
	}
	int irroe=WSAGetLastError();
	return  TRUE;

}
BOOL CMTJJAEquipment::SendWebCommand(CString szEquimentKey, const CString &szCommand)
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
	if(szCommand.IsEmpty())
		return  FALSE;
	sprintf(bufcmd,"%s",szCommand);
	int i=sendto(fd,bufcmd,strlen(bufcmd),0,(SOCKADDR *)&addr,sizeof(pNetInfo->addr));
	if(i<=0)
	{
		
		return FALSE;
	}
	int irroe=WSAGetLastError();
	//en
	__SendLog(bufcmd);
	return  TRUE;
}


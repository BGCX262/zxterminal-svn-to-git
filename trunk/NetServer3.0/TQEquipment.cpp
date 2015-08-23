// TQEquipment.cpp: implementation of the CTQEquipment class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "TQEquipment.h"
#include "DBWork.h"
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
extern CDBWork  g_DBWork;
#define TQMAXCOMMANDID  8
#define   MINLENTCP    19  
#define   MINLENUDP    20

CString   szReType2="I1,";  //��Ҫȷ�ϵ���ӳ��Ϣ
CString   szReType3="";  //��Ҫȷ�ϵ���ӳ��Ϣ


void      _Log(CString szLog)
{
	static  int icount=0;
	COleDateTime    now=COleDateTime::GetCurrentTime();
	CString  szTemp;
	szTemp+=szLog;
	CString    szFileName;
	szFileName.Format("Log\\TQ%s.log",now.Format("%Y%m%d"));
	ofstream   f(szFileName.GetBuffer(0),ios::app|ios::out);
	
#ifdef  _DEBUG
	//������ָ����¼�����ڵ��Ե�ʱ��Ĺ���;
	char buf[4096]={0};
	sprintf(buf,"%s",szTemp);
    f<<buf<<endl;
	f.close();
	return ;
#endif	
    f<<szTemp.GetBuffer(0)<<endl;
	f.close();
	return ;
	
}
void   __TQSendGpsLog(char  * buf)
{
	ofstream  f("TQSendGpsLog.Log",ios::app| ios::out);
	f<<buf<<endl;
	f.close();
}
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



CTQEquipment::CTQEquipment()
{

}

CTQEquipment::~CTQEquipment()
{

}
BOOL   CTQEquipment::IsDataLegalAndFindPackNum(char *p,int  iLen,int& INumber,int &iEnd)
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
BOOL   CTQEquipment::IsRegiste(CIOCPBuff  *pIOBuf)
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

	if(pData[15]=='I' && pData[16]=='1')
		return  TRUE;
	return  TRUE;

}
BOOL CTQEquipment::ParseGpsData(CIOCPBuff *pIOBuf,CGPSData  &GpsData,CString &szMessage)
{
	if(!pIOBuf)
		return FALSE;
	int iNumber=0,iEnd=pIOBuf->iMsgLen;
    if(!IsDataLegalAndFindPackNum(pIOBuf->buf,pIOBuf->iMsgLen,iNumber,iEnd))
		return  FALSE;
	if(IsRegiste(pIOBuf))
	{//�����豸��ע�᣻��Ҫ�Ǽ�¼�豸��������Ϣ��
        
		CString   szKey;		 
		char  *p=pIOBuf->GetWSABuffer()->buf;
		char   temp[15]={0};
		memcpy(temp,&p[4],10);
		szKey=temp;
		this->m_EquimenList.UpdatUser(pIOBuf->fd,szKey,pIOBuf->addr);		
	}
	return  DecGpsData(pIOBuf,GpsData,szMessage);

}

BOOL CTQEquipment::DecGpsData(CIOCPBuff *p, CGPSData &GpsData,CString &szMessage)
{
	if(!p)
		return  FALSE;
   char *pData=p->buf;
   if(!pData)
	   return FALSE;
   TQV1Data *pTQV1Data=NULL;
   TQV42Data *pTQV42Data=NULL;
   TQV43Data *pTQV43Data=NULL;
   if(pData[0]=='*')
   {//δѹ��������;
	   CString  szPp=pData;
       int   SubTypeLen=0;
	   TQDATATYPE iType=QueryDataType(pData,p->iMsgLen,SubTypeLen);
	   switch(iType) 
	   {
	   case  TYPE_V1:
		   {//һ���������Ϣ;
			   pTQV1Data=(TQV1Data * )(pData=p->buf);
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
				   return FALSE;
			   }
				         
			   //����Ե�ʱ���õ�
		//	   COleDateTime  now=COleDateTime::GetCurrentTime();
			   COleDateTime now=GetNOYSTime(szPp);
			   CString     szTime=now.Format("%Y-%m-%d %H:%M:%S");
			   sprintf(GpsData.Time,"%s",szTime);
		//	   GpsData.Time[20]=0;
			   //end;

			   memcpy(GpsData.szEquimentID,pTQV1Data->szKey,10);  //�õ��豸��key;
			   if(this->GetTaxibeWork(GpsData.szEquimentID)==TRUE)
			   {
				   GpsData.iState=GPS_STATONE;
			   }
		   }
	   	break;
	   case TYPE_V4:
		   {
			   if(SubTypeLen==2)
			   {
				   
				   pTQV42Data=(TQV42Data * )(pData=p->buf);
				   pTQV42Data->init();
				   if(szReType2.Find(pTQV42Data->TYPE2)==-1)
					   return  FALSE;  //����һ��ʼ����ľͲ������;
				   GpsData.fLongitude=DecLONG(pTQV42Data->Longtinue);
				   
				   GpsData.fLatitude=DecLat(pTQV42Data->Lat);
				   GpsData.fSpeed=(float)atof(pTQV42Data->Speed);
				   GpsData.iDirection=atoi(pTQV42Data->Diredtion);
				   GpsData.iState=SetState(pTQV42Data->state);
				   if(pTQV42Data->Able[0]='A')
					   GpsData.iFlag=6;
				   else
				   {
					   return FALSE;
					   GpsData.iFlag=0; 
				   }
				   //����Ե�ʱ���õ�
				//   COleDateTime  now=COleDateTime::GetCurrentTime();
				   COleDateTime now=GetNOYSTime(szPp);
				   CString     szTime=now.Format("%Y-%m-%d %H:%M:%S");
				   sprintf(GpsData.Time,"%s",szTime);
				   memcpy(GpsData.szEquimentID,pTQV42Data->szKey,10);  //�õ��豸��key;
                   //�жϻ�Ӧ��Ϣ������;
				   CString  szReMessage=pTQV42Data->TYPE2;
				   if(szReMessage.Find("I1")!=-1)
				   {//����Ϣ��ȷ����;
					   GpsData.iState=GPS_SENDMESSAGEOK;
				   }			   
			   }
			   if(SubTypeLen==3)
			   {
				   pTQV43Data=(TQV43Data * )(pData=p->buf);
				   pTQV43Data->init();
			   }
			  // �豸��Ӧ����Ϣ
	
		   }
	   	break;
	   case  TYPE_I1:
		   {
			   //����Ϣ
			   DOTQMessage(pData,p->iMsgLen,szMessage);
			   memcpy(GpsData.szEquimentID,pData+4,10);  //�õ��豸��key;
			   GpsData.iFlag=0;
			   COleDateTime  now=COleDateTime::GetCurrentTime();
			   CString     szTime=now.Format("%Y-%m-%d %H:%M:%S");
			   sprintf(GpsData.Time,"%s",szTime);
		   }
		   break;
		    
	   case  TYPE_I8:
		   { //�ӼƼ�������������Ϣ;

			   CString  szStr;
			   if(GetDCBTOASC(pData,p->iMsgLen,szStr)==FALSE)
				   return  FALSE;
			  memcpy(GpsData.szEquimentID,pData+4,10);  //�õ��豸��key;
              szMessage=GetTaxiData(GpsData.szEquimentID,szStr);
			  GpsData.iFlag=0;
			  COleDateTime  now=COleDateTime::GetCurrentTime();
			  CString     szTime=now.Format("%Y-%m-%d %H:%M:%S");
			  sprintf(GpsData.Time,"%s",szTime);
			   //װ��
			  if(szMessage.Find("�س�")!=-1)  
			  {
				  this->SetTaxistate(GpsData.szEquimentID,TRUE);
			  }
			  //�¿���;
			  if(szMessage.Find("�ճ�")!=-1  ||  szMessage.Find("������Ϣ")!=-1)
			  {				  
				  this->SetTaxistate(GpsData.szEquimentID,FALSE);
			  }
		   }
		   break;
	   default:
		   ASSERT(FALSE);
		   return FALSE;
	   }	   
	   

   }
   //$3070320141084211300407  3116853500 121278339E000168FBFFFBFFFF0048
   if(pData[0]=='$')
   {//ѹ��������;
      TQPREDATA  *pPreData=NULL;
	   pPreData=(TQPREDATA * )(pData);
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
			//�ٶȺͷ���;
			CString  szSpeed=szStr.Mid(Eindex+1,3);
			CString  szOrinet=szStr.Mid(Eindex+4,3);
			GpsData.fSpeed=atoi(szSpeed.GetBuffer(0)) *1.852;
            GpsData.iDirection=atoi(szOrinet.GetBuffer(0));

	   }

	   else
	   {
		   return FALSE;
		   GpsData.iFlag=0; 
	   }
		   

	   
	   //����Ե�ʱ���õ�
	//   COleDateTime  now=COleDateTime::GetCurrentTime();
	   COleDateTime  now=GetYSTime(szStr);
	   CString     szTime=now.Format("%Y-%m-%d %H:%M:%S");
	   sprintf(GpsData.Time,"%s",szTime);   
	   memcpy(GpsData.szEquimentID,szId.GetBuffer(0),10);
	   if(this->GetTaxibeWork(GpsData.szEquimentID)==TRUE)
	   {
		   GpsData.iState=GPS_STATONE;
	   }
   }
	return TRUE;
}

//�ж����ݵ�����;
TQDATATYPE CTQEquipment::QueryDataType(char  *p,int ilen,int &SubTypeLen)
{
	if(!p)
		return TYPE_TQNULL; 
	if(p[0]=='$')
		return  TYPE_PRESS;   //ѹ�����ݵı�־;
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
UINT CTQEquipment::SetState(char *pState)
{
	return  0;

}
//BCD��ת����ASc��
BOOL CTQEquipment::GetDCBTOASC(char *buf, int iLen, CString &szStr)
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

//�������е�����ĺ���;
BOOL CTQEquipment::SendTQGPSCommand(CString szEquimentKey, const CGpsCommand *pCommand)
{
	CString  szLog;
	szLog.Format("CMTGPSEquipment::SendTQGPSCommand begin  %s  ",szEquimentKey);
	char  logbuf[1000]={0}; 
	sprintf(logbuf,"%s",szLog);
	__TQSendGpsLog(logbuf);
	
	if(!pCommand)
		return FALSE;
	EquiNetInfo *pNetInfo=this->m_EquimenList.Find(szEquimentKey);
	//��������
	if(!pNetInfo)
	{
		szLog="CTQEquipment::SendMTGPSCommand pNetInfo  ==NULL  return  FALSE ";
		memset( logbuf,0,1000);
		sprintf(logbuf,"%s",szLog);
		__TQSendGpsLog(logbuf);
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
		__TQSendGpsLog(logbuf);
		return  FALSE;
	}
	char bufcmd[1024];
	if(!PackCommand(pCommand,bufcmd,1024))
	{
		szLog="CTQEquipment::SendTQGPSCommand ���ܽ�����������  return  FALSE ";
		memset( logbuf,0,1000);
		sprintf(logbuf,"%s",szLog);
		__TQSendGpsLog(logbuf);
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
		__TQSendGpsLog(logbuf);
		
		return FALSE;
	}
	int irroe=WSAGetLastError();
	//en
	__TQSendGpsLog(bufcmd);
	return  TRUE;
}
BOOL CTQEquipment::PackCommand(const CGpsCommand *pCommand,char buf[],int iLen)
{
	CString  szCommandinfo[8]={"����","�������","����ȷ��","�������","����Ϣ","�ϱ�ʱ������",
		"���ٶ�����","��������"};

	CString  szEquimentKey=pCommand->EquimentId;
	if(szEquimentKey.IsEmpty())
		return  FALSE;
	int  iCommandID=pCommand->iCOmmandID;
	if(iCommandID==USER_DEFINED)
	{//�û��Զ��������
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
	//*FFFFFFFFFFF,PPPPPP,XXXX,ZZ��ZZ#CC
	
	CString szCommandText=szCommandinfo[iCommandID];
	if(szCommandText.IsEmpty())
		return  FALSE;
	CString  szCommand;  
	if(szCommandText.Find("�ϱ�ʱ������")!=-1)
	{
		//*XX,YYYYYYYYYY,D1,HHMMSS,interval,batch#
		CString  szPWS=pCommand->Param;
		int  itv=atoi(szPWS.GetBuffer(0));
		if(itv<=0 ||itv>=255)
			return  FALSE;
		szCommand.Format("*HQ,000,D1,121212,%d,1#",itv);
	   g_DBWork.UpdateEquimentTimeSet(szEquimentKey,szCommand);
	//	szCommand.Format("*HQ,000,S2,121212,111#");


	}
	if(szCommandText.Find("����Ϣ")!=-1)
	{
		//*XX,YYYYYYYYYY,I1,HHMMSS,Display_Time,Code,Info_lenth,Information
       CString  szPAR=pCommand->Param;
		int  iLen;
        return PackSMSCommand(buf,iLen,szPAR);
			
	}
	memset(buf,0,iLen);
	sprintf(buf,"%s",szCommand);
	return  TRUE;	

}


//��������Ϣ��*XX,YYYYYYYYYY,I1,HHMMSS,1,Code,Info_lenth,Information#
BOOL CTQEquipment::DOTQMessage(char *pBuf, int iLen,CString  &szMessage)
{
    if(!pBuf)
		return  FALSE;
	if(iLen<sizeof(TQMESSAGE))
		return  FALSE;

	CString  szStr=pBuf;
	if(szStr.GetLength() <  sizeof(TQMESSAGE))
		return  FALSE;
	
	int  index=szStr.Find(",",sizeof(TQMESSAGE));
	if(index==-1)
		return  FALSE;
	CString  szino_Len=szStr.Mid(sizeof(TQMESSAGE),index-sizeof(TQMESSAGE));
	int  iiLen=atoi(szino_Len.GetBuffer(0));
	if(iiLen<=0)
		return  FALSE;

	//��ֲ����Ϣ��
	char  tempbuf[100]={0};
	memcpy(tempbuf,&pBuf[index+1],iiLen);
	if(tempbuf[0]=='1'  &&  tempbuf[1]=='8')
	{//ok����
		szMessage="����";
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


BOOL CTQEquipment::PackSMSCommand(char *pBuf, int &iLen, CString &szMessage)
{

	CString szMsg;
	int istrlen=szMessage.GetLength();
	char bufmessage[1024]={0};
	sprintf(bufmessage,"%s",szMessage);
	//�������ֵĴ���
	char  *buftemp=NULL;
	buftemp=new char[1024];
	
	int icount=0;
	if(buftemp==NULL)
		return  -1;
	memset(buftemp,0,1024);
	//�жϷǺ��ֵ���
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
			//�Ǻ���
			memset(testbuf,0,5);
			sprintf(testbuf+1,"%s",sztempMsg);
		}
		else
		{
			//����
			sprintf(testbuf,"%s",sztempMsg);
		}
		//д���ڴ�;
		memcpy(buftemp+icount,testbuf,2);
		icount+=2;
	}  
	if(icount>256)
	{
		icount=256;
	}
	szMsg.Format("*HQ,000,I1,121212,60000,0,%d,",icount);
//	char  buf[1024]={0};
	sprintf(pBuf,"%s",szMsg);
	int iBegin=strlen(pBuf);
	memcpy(pBuf+iBegin,buftemp,icount);
	char endsign[2]="#";
	memcpy(pBuf+iBegin+icount,endsign,1);
	iLen=iBegin+icount+1;
	return  TRUE;

}

CString CTQEquipment::GetTaxiData(CString  szKey,CString szStr)
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
		return   szTemp;   //��		  
	}
	// 	BF 02 30 30 BF 03   �س�;
	//  BF 02 34 34 BF 03   �ճ�
	//BF 02 36 20 02 12 07 15 41 12 07 15 43 02 70 00 00 36 00 00 10 00 00 00 00 20 00 00 19 00 20 47 1C BF 03 ����
//    this->SendAtack(szKey);
	szLog=szLog.Mid(Bindex,Eindex-Bindex+4);
	if(szLog.GetLength()==12)
	{
		//�ճ�  and  �س�;
		if(szLog.Find("3030",4)!=-1)
		{//�س�
			szTemp="�س�";
			
			return  szTemp;
		}
		if(szLog.Find("3434",4)!=-1)
		{//
			szTemp="�ճ�";
			
			return  szTemp;
		}
	}
	else
	{//��������;
		//BF0236  20 02 12 07 15 41 12 07 15 43 02 70 00 00 36 00 00 10 00 00 00 00 20 00 00 19 00 20 47 1C BF 03 ����
		if(szLog.Find("36")!=4)
		{
			szTemp="?";
			return  szTemp;   //edited  20061219;  //��������;
		}
		CString szUpTime=szLog.Mid(6,12);//�ϳ�ʱ��
		CString  szDownTime=szLog.Mid(18,8);//�³�ʱ��;
		CString  szprice=szLog.Mid(26,4);//���۸�;
		CString  szDidtance=szLog.Mid(30,6);//���
		CString  szwaiteTime=szLog.Mid(36,6);  //�Ⱥ�ʱ��
        CString  szleisureTime=szLog.Mid(42,6);//����ʱ��
		CString  szsum=szLog.Mid(48,6);//���;
		//����Щ���ݽ��д���;
		CString  szTemp;
		szTemp.Format("%s-%s-%s %s:%s:%s",szUpTime.Mid(0,2),szUpTime.Mid(2,2),szUpTime.Mid(4,2),szUpTime.Mid(6,2),szUpTime.Mid(8,2),szUpTime.Mid(10,2));		
		CString   szSCSJ=szTemp;//�ϳ�ʱ��
		CString   szXCSJ;
		szTemp.Empty();
		szTemp.Format("%s-%s %s:%s",szDownTime.Mid(0,2),szDownTime.Mid(2,2),szDownTime.Mid(4,2),szDownTime.Mid(6,2));
		szXCSJ=szTemp;//�³�ʱ��;
		int i=atoi(szprice.GetBuffer(0));
		float  iPrice=((float)i)/100;//���۸�;
		float  fDistace=(atof(szDidtance.GetBuffer(0)))/100;//���
		szTemp.Empty();
		szTemp.Format("%s:%s:%s",szwaiteTime.Mid(0,2),szwaiteTime.Mid(2,2),szwaiteTime.Mid(4,2));
		szwaiteTime=szTemp;//�Ⱥ�ʱ��
		szTemp.Empty();
		szTemp.Format("%s:%s:%s",szleisureTime.Mid(0,2),szleisureTime.Mid(2,2),szleisureTime.Mid(4,2));
		szleisureTime=szTemp;//����ʱ��
		float   fqian=(atof(szsum.GetBuffer(0)))/100;//���;
		szTemp.Empty();
		szTemp.Format("������Ϣ:�ϳ�ʱ��:%s  �³�ʱ��:%s ����%.2fԪ ���%.1fkm �Ⱥ�ʱ��%s ����ʱ��%s  ���%.2fԪ ",szSCSJ,szXCSJ,iPrice,fDistace,szwaiteTime,szleisureTime,fqian);
		return  szTemp;
	}
}


void CTQEquipment::SetTaxistate(CString szKey,BOOL  beWork)
{

	szKey.TrimLeft(),szKey.TrimRight();
	if(szKey.IsEmpty())
		return;
	int  iCount=m_TaxiWorkArray.GetSize();

	if(beWork==TRUE)
	{//װ�ͽ׶�
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

BOOL CTQEquipment::GetTaxibeWork(CString szKey)
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


//�������е�����ĺ���;
BOOL CTQEquipment::SendTQGPSCommandStr(CString szEquimentKey,CString szStr)
{
	CString  szLog;
	szLog.Format("CMTGPSEquipment::SendTQGPSCommand begin  %s  ",szEquimentKey);
	char  logbuf[1000]={0}; 
	sprintf(logbuf,"%s",szLog);
	__TQSendGpsLog(logbuf);
	EquiNetInfo *pNetInfo=this->m_EquimenList.Find(szEquimentKey);
	//��������
	if(!pNetInfo)
	{
		szLog="CTQEquipment::SendMTGPSCommand pNetInfo  ==NULL  return  FALSE ";
		memset( logbuf,0,1000);
		sprintf(logbuf,"%s",szLog);
		__TQSendGpsLog(logbuf);
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
		__TQSendGpsLog(logbuf);
		return  FALSE;
	}
	char bufcmd[1024]={0};

	//  CString  szTemp="*13370270190,000000,S01010111#ff";
	//  sprintf(bufcmd,"%s",szTemp);
	// int i=m_SendSock.SendTo(bufcmd,strlen(bufcmd),(SOCKADDR *)&addr,sizeof(pNetInfo->addr),0);
	
	// Sendfd
	//	int i=sendto(fd,bufcmd,strlen(bufcmd),0,(SOCKADDR *)&addr,sizeof(pNetInfo->addr));
    sprintf(bufcmd,"%s",szStr);
	int i=send(fd,bufcmd,szStr.GetLength(),0);
	
	if(i<=0)
	{
		szLog="i<=0  return  FALSE ";
		memset( logbuf,0,1000);
		sprintf(logbuf,"%s",szLog);
		__TQSendGpsLog(logbuf);
		
		return FALSE;
	}
	int irroe=WSAGetLastError();
	//en
	__TQSendGpsLog(bufcmd);
	return  TRUE;
}
//�õ�ʱ��gpsʱ��;
COleDateTime  CTQEquipment::GetNOYSTime(const CString  &szData)
{
	//ȡ��tcp��ʱ��;
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
	szDate=szData.Mid(iIndex-15,6);  //����;
	CString  szHH=szTime.Mid(0,2);
	//����16
	int  iHH=atoi(szHH.GetBuffer(0));
	if(iHH<=15)  
	{//�����Ĵ���;
		CString  sztemp;
		sztemp.Format("20%s-%s-%s %d:%s:%s",szDate.Mid(4,2),szDate.Mid(2,2),szDate.Mid(0,2),atoi(szTime.Mid(0,2))+8,szTime.Mid(2,2),szTime.Mid(4,2));
		COleDateTime  now;
		now.ParseDateTime(sztemp);
		return  now;	
	}
	if(iHH>=16)  //����ڶ���ģ�
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


COleDateTime  CTQEquipment::GetYSTime(const CString  &szData)
{	//ȡ��udp��ʱ��;
	COleDateTime  now1=COleDateTime::GetCurrentTime();
	CString  szTime;	
	szTime=szData.Mid(11,6);
	CString  szHH=szTime.Mid(0,2);
	//����16
	int  iHH=atoi(szHH.GetBuffer(0));
	if(iHH<=15)
	{//�����Ĵ���;
		CString  szDate;
		szDate=szData.Mid(17,6);
		CString  sztemp;
		sztemp.Format("20%s-%s-%s %d:%s:%s",szDate.Mid(4,2),szDate.Mid(2,2),szDate.Mid(0,2),atoi(szTime.Mid(0,2))+8,szTime.Mid(2,2),szTime.Mid(4,2));
		COleDateTime  now;
		now.ParseDateTime(sztemp);
		return  now;	
	}
	if(iHH>=16)  //����ڶ���ģ�
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



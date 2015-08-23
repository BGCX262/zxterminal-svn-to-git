// DBWork.cpp: implementation of the CDBWork class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetServer.h"
#include "DBWork.h"
#include "Log.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#include "ReturnNumber.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern  CNetServerApp  theApp;
extern  void      g_Log(CString szLog);

int CDBWork::DbCount=0;
CDBWork::CDBWork()
{

	DbCount++;
}

CDBWork::~CDBWork()
{

}

int CDBWork::ConnectDB(CString szDataBaseName, CString szUser, CString szPws)
{
	if(szDataBaseName.IsEmpty())
		return  DB_PAR_ERROR; 
	this->DisConnectDB();
	CString  szConnection;	
	//aql
	//Provider=SQLOLEDB.1;Integrated Security=SSPI;Persist Security Info=False;Initial Catalog=NETDB;Data Source=MINGTIAN-27XYS0
	//Provider=SQLOLEDB.1;Password=sa;Persist Security Info=True;User ID=sa;Initial Catalog=NETDB;Data Source=TGCJC
//	szConnection.Format("Provider=OraOLEDB.Oracle.1;Password=DBUSER;Persist Security Info=True;User ID=DBUSER;Data Source=wmgpsdb");
//	szConnection.Format("Provider=SQLOLEDB.1;Integrated Security=SSPI;Persist Security Info=False;Initial Catalog=NETDB;Data Source=MINGTIAN-27XYS0");
//	szConnection.Format("Provider=OraOLEDB.Oracle.1;Password=SZGPS;Persist Security Info=True;User ID=SZGPS;Data Source=GPSDB");
//	szConnection.Format("Provider=OraOLEDB.Oracle.1;Password=NETDB271715;Persist Security Info=True;User ID=TESt271715;Data Source=NETDB");
//	szConnection.Format("Provider=OraOLEDB.Oracle.1;Password=Zt1026;Persist Security Info=True;User ID=ZTNET;Data Source=NETDB");
//	szConnection.Format("Provider=SQLOLEDB.1;Password=sasa;Persist Security Info=True;User ID=sa;Initial Catalog=NETDB;Data Source=218.87.216.71");
 	szConnection.Format("Provider=SQLOLEDB.1;Integrated Security=SSPI;Persist Security Info=False;Initial Catalog=NETDB;Data Source=IT-D0395");
	//szConnection.Format("Provider=SQLOLEDB.1;Initial Catalog=NETDB;Data Source=220.176.195.200");
//	szConnection.Format("Provider=SQLOLEDB.1;Integrated Security=SSPI;Persist Security Info=False;Initial Catalog=NETDB;Data Source=sid-work1");
//	szConnection.Format("Provider=SQLOLEDB.1;Password=GPS2010)&)%%Server;Persist Security Info=True;User ID=sa;Initial Catalog=NETDB;Data Source=218.87.216.71");
	
	try
	{
		if(!m_Database.Open(szConnection, "sa", "GPS2010)&)%Server"))
		{
			
			CString  szLog;
			szLog.Format("数据库连接失败szConnection=%s",szConnection);
			g_Log(szLog);
			return  DB_EXE_ERROR;  //数据库语句执行错误
		}
	}
	catch (_com_error  e)
	{
		CString  szTemp;
		szTemp.Format("%s",(char *)e.Description());
		g_Log(szTemp);
		return  DB_EXE_ERROR;
	}
	return  DB_OK;
}



int CDBWork::DisConnectDB()
{
	if(m_Database.IsOpen())
		m_Database.Close();
	return  DB_OK;
}


BOOL  CDBWork::IsUserlegal(CString   szUser,CString  szPws)
{	

	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  USERIINFO  where USERID='%s' and PASSWORD='%s'",szUser,szPws);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("验证用户合法性失败.(SQL:%s)", szSQL);
			g_Log(szTemp);
			return FALSE;
		}
		if(!rst.IsEof())
		{
			rst.Close();
			return  TRUE;
		}
		else
			return FALSE;
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("验证用户合法性失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return FALSE;
	}	
	return TRUE;
}

int CDBWork::GetUserAllEquiment(CString  & szUser,CStringArray &asAllEquiMent)
{
	if(szUser.IsEmpty())
		return  DB_PAR_ERROR;
	asAllEquiMent.RemoveAll();
	asAllEquiMent.SetSize(0);
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  USERSEQUIMENT  where USERID='%s'",szUser);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到用户的设备信息失败.(SQL:%s)", szSQL);
			g_Log(szTemp);
			return FALSE;
		}
		
		while(!rst.IsEof())
		{
			rst.GetFieldValue("EQUIKEY",szTemp);
			szTemp.TrimLeft(),szTemp.TrimRight();
			if(!szTemp.IsEmpty())
				asAllEquiMent.Add(szTemp);
			rst.MoveNext();
		}
		rst.Close();

		
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到用户的设备信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return FALSE;
	}	
	return TRUE;

}

int CDBWork::GetEquimentInfo(const CString &szEquimentId, CEquimentInfo &EquiInfo)
{
	if(szEquimentId.IsEmpty())
		return  DB_PAR_ERROR;
	CString szSQL, szTemp;
	int  EquiType;
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  EQUIMENTINFO  where EQUIMENTID='%s'",szEquimentId);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到设备%s信息失败.(SQL:%s)", szEquimentId,szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		if(!rst.IsEof())
		{
			rst.GetFieldValue("EQUIMENTKEY",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(EquiInfo.EquimentKey,"%s",szTemp);
				EquiInfo.EquimentKey[szTemp.GetLength()]=0;
			}
			//
			rst.GetFieldValue("EQUIMENTNUMBER",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(EquiInfo.EQUIMENTNUMBER,"%s",szTemp);
				EquiInfo.EQUIMENTNUMBER[szTemp.GetLength()]=0;
			}
			
			rst.GetFieldValue("INDUSTRYNUMBER",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(EquiInfo.INDUSTRYNUMBER,"%s",szTemp);
				EquiInfo.INDUSTRYNUMBER[szTemp.GetLength()]=0;
			}

			rst.GetFieldValue("OWNERNAME",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(EquiInfo.OWNERNAME,"%s",szTemp);
				EquiInfo.OWNERNAME[szTemp.GetLength()]=0;
			}
			rst.GetFieldValue("USERNAME1",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(EquiInfo.USERNAME1,"%s",szTemp);
				EquiInfo.USERNAME1[szTemp.GetLength()]=0;
			}
			rst.GetFieldValue("USERNAME2",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(EquiInfo.USERNAME2,"%s",szTemp);
				EquiInfo.USERNAME2[szTemp.GetLength()]=0;
			}

			rst.GetFieldValue("PHONE",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(EquiInfo.PHONE,"%s",szTemp);
				EquiInfo.PHONE[szTemp.GetLength()]=0;
			}
			rst.GetFieldValue("REMARK",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(EquiInfo.REMARK,"%s",szTemp);
				EquiInfo.REMARK[szTemp.GetLength()]=0;
			}
			//EquiType
			rst.GetFieldValue("EQUIMENTMAINTYPE",EquiType);
			EquiInfo.EquimentMainType=(UINT)EquiType;
			rst.GetFieldValue("EQUIMENTSUBTYPE",EquiType);
			EquiInfo.EquimentSubType=(UINT)EquiType;
	
		}
		rst.Close();
		
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到设备%s信息失败.(Description:%s, SQL:%s)",
			szEquimentId,(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
}

int CDBWork::GetEquipmentType(const CString szEquimentId, int& nCarType, int& nIsRun)
{
	if(szEquimentId.IsEmpty())
		return  DB_PAR_ERROR;

	CString szSQL, szTemp;
	int nValue;
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  EQUIMENTINFO  where EQUIMENTID='%s'", szEquimentId);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到设备%s信息失败.(SQL:%s)", szEquimentId, szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}

		if(!rst.IsEof())
		{
			
			rst.GetFieldValue("CARTYPE", nValue);
			nCarType = nValue;

			rst.GetFieldValue("ISRUN", nValue);
			nIsRun = nValue;
		}

		rst.Close();
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到设备%s信息失败.(Description:%s, SQL:%s)", szEquimentId, (char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}

	return  DB_OK;
}


//设备的信息;
int CDBWork::GetEquimentInfo(CObArray &asEquiment)
{

	CString szSQL, szTemp;
	int  EquiType;
	CEquimentInfo  *pEquiInfo=NULL;
	asEquiment.RemoveAll();
	asEquiment.SetSize(0);
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  EQUIMENTINFO");		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到设备信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		while(!rst.IsEof())
		{
			pEquiInfo = new CEquimentInfo;
			if(!pEquiInfo)
				continue;
			rst.GetFieldValue("EQUIMENTID",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(pEquiInfo->EquimentId,"%s",szTemp);
				pEquiInfo->EquimentId[szTemp.GetLength()]=0;
			}

			rst.GetFieldValue("EQUIMENTKEY",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(pEquiInfo->EquimentKey,"%s",szTemp);
				pEquiInfo->EquimentKey[szTemp.GetLength()]=0;
			}
			//
			rst.GetFieldValue("EQUIMENTNUMBER",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(pEquiInfo->EQUIMENTNUMBER,"%s",szTemp);
				pEquiInfo->EQUIMENTNUMBER[szTemp.GetLength()]=0;
			}
			
			rst.GetFieldValue("INDUSTRYNUMBER",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(pEquiInfo->INDUSTRYNUMBER,"%s",szTemp);
				pEquiInfo->INDUSTRYNUMBER[szTemp.GetLength()]=0;
			}

			rst.GetFieldValue("OWNERNAME",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(pEquiInfo->OWNERNAME,"%s",szTemp);
				pEquiInfo->OWNERNAME[szTemp.GetLength()]=0;
			}
			rst.GetFieldValue("USERNAME1",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(pEquiInfo->USERNAME1,"%s",szTemp);
				pEquiInfo->USERNAME1[szTemp.GetLength()]=0;
			}
			rst.GetFieldValue("USERNAME2",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(pEquiInfo->USERNAME2,"%s",szTemp);
				pEquiInfo->USERNAME2[szTemp.GetLength()]=0;
			}

			rst.GetFieldValue("PHONE",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(pEquiInfo->PHONE,"%s",szTemp);
				pEquiInfo->PHONE[szTemp.GetLength()]=0;
			}
			rst.GetFieldValue("REMARK",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(pEquiInfo->REMARK,"%s",szTemp);
				pEquiInfo->REMARK[szTemp.GetLength()]=0;
			}
			//EquiType
			rst.GetFieldValue("EQUIMENTMAINTYPE",EquiType);
			pEquiInfo->EquimentMainType=(UINT)EquiType;
			rst.GetFieldValue("EQUIMENTSUBTYPE",EquiType);
			pEquiInfo->EquimentSubType=(UINT)EquiType;
	
			asEquiment.Add((CObject *)pEquiInfo);
			rst.MoveNext();	
		}
	
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到设备信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

int CDBWork::Free_EquimentInfo(CObArray &asEquimentInfo)
{

	int iCount=asEquimentInfo.GetSize();
	CEquimentInfo  *pEquiInfo=NULL;
	for(int iLoop=0;iLoop<iCount;iLoop++)
	{

		pEquiInfo=(CEquimentInfo  *)asEquimentInfo.GetAt(iLoop);
		delete  pEquiInfo;
		pEquiInfo=NULL;
	}
	asEquimentInfo.RemoveAll();
	asEquimentInfo.SetSize(0);
	return  DB_OK;
}


//在数据库里查找设备的信息;
int CDBWork::GetEquimentInfoID(const CString &szKey,CString &szEquimentId, BOOL bMemory)
{
	BOOL  bFind=FALSE;
	if(bMemory)
	{//在内存中读取
		theApp.m_AllEquimentInfoLock.Lock();
		int iCOunt=theApp.m_asEquimentInfo.GetSize();
		for(int i=0;i<iCOunt;i++)
		{
			CEquimentInfo *p=(CEquimentInfo *)theApp.m_asEquimentInfo[i];

			if(szKey==p->EquimentKey)
			{//找到新匹配的数据;
				szEquimentId=p->EquimentId;
				theApp.m_AllEquimentInfoLock.Unlock();
			     
				return  DB_OK;
			}
		}
		theApp.m_AllEquimentInfoLock.Unlock();
		//		return  DB_EXE_ERROR;

	}
	if(bFind==FALSE)
	{//没有找到就到数据库找;
		CEquimentInfo   *pinfo=new CEquimentInfo;
		if(!pinfo)
		{
			return  DB_EXE_ERROR;

		}
		if(this->GetEquimentInoFromid(szKey,  *pinfo)<0)
			return   DB_EXE_ERROR;
		szEquimentId=pinfo->EquimentId;
		theApp.m_AllEquimentInfoLock.Lock();
		theApp.m_asEquimentInfo.Add((CObject  *)pinfo);
		theApp.m_AllEquimentInfoLock.Unlock();
		return  DB_OK;
	}

}

//用设备的机器号得到设备的信息;
int CDBWork::GetEquimentInoFromid(const CString &szKey,CEquimentInfo  &Info)
{
		if(szKey.IsEmpty())
		return  DB_PAR_ERROR;
	CString szSQL, szTemp;
	int  EquiType;
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  EQUIMENTINFO  where EQUIMENTKEY='%s'",szKey);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到设备%s信息失败.(SQL:%s)", szKey,szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		if(!rst.IsEof())
		{

			rst.GetFieldValue("EQUIMENTID",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(Info.EquimentId,"%s",szTemp);
				Info.EquimentId[szTemp.GetLength()]=0;
			}


			rst.GetFieldValue("EQUIMENTKEY",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(Info.EquimentKey,"%s",szTemp);
				Info.EquimentKey[szTemp.GetLength()]=0;
			}
			//
			rst.GetFieldValue("EQUIMENTNUMBER",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(Info.EQUIMENTNUMBER,"%s",szTemp);
				Info.EQUIMENTNUMBER[szTemp.GetLength()]=0;
			}
			
			rst.GetFieldValue("INDUSTRYNUMBER",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(Info.INDUSTRYNUMBER,"%s",szTemp);
				Info.INDUSTRYNUMBER[szTemp.GetLength()]=0;
			}

			rst.GetFieldValue("OWNERNAME",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(Info.OWNERNAME,"%s",szTemp);
				Info.OWNERNAME[szTemp.GetLength()]=0;
			}
			rst.GetFieldValue("USERNAME1",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(Info.USERNAME1,"%s",szTemp);
				Info.USERNAME1[szTemp.GetLength()]=0;
			}
			rst.GetFieldValue("USERNAME2",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(Info.USERNAME2,"%s",szTemp);
				Info.USERNAME2[szTemp.GetLength()]=0;
			}

			rst.GetFieldValue("PHONE",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(Info.PHONE,"%s",szTemp);
				Info.PHONE[szTemp.GetLength()]=0;
			}
			rst.GetFieldValue("REMARK",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(Info.REMARK,"%s",szTemp);
				Info.REMARK[szTemp.GetLength()]=0;
			}
			//EquiType
			rst.GetFieldValue("EQUIMENTMAINTYPE",EquiType);
			Info.EquimentMainType=(UINT)EquiType;
			rst.GetFieldValue("EQUIMENTSUBTYPE",EquiType);
			Info.EquimentSubType=(UINT)EquiType;
	
		}
		rst.Close();
		
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到设备%s信息失败.(Description:%s, SQL:%s)",
			szKey,(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
}
//end;  

int CDBWork::UpdataGPSData(CString szTableName,const CGPSData &pData)
{

	
	CString   szSql,szTemp;
//	CADORecordset rst(&m_Database);
	szSql.Format("insert into %s(EquimentID,DataTime,longitude,latitude,SPEED,Direction,State,FLAG)"
		"  values('%s','%s',%f,%f,%f,%d,%d,%d)",
		szTableName,pData.szEquimentID,pData.Time,pData.fLongitude,pData.fLatitude,pData.fSpeed,pData.iDirection,pData.iState,pData.iFlag);
	try
	{
		if(!m_Database.Execute(szSql))
		{
			CreateTable(szTableName);
			szTemp.Format("写入GPSDATA表失败信息失败.(SQL:%s)",szSql);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch (_com_error  &e)
	{
		CreateTable(szTableName);
		szTemp.Format("写入GPSDATA表失败信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSql);
		g_Log(szTemp);
		return DB_EXE_ERROR;
		
	}
	return DB_OK;
}




//得到设备的归属用户;
int CDBWork::GetEquimentUser(CString szEquimentId, CStringArray &aUsers)
{
	CString szSQL, szTemp;
	if(szEquimentId.IsEmpty())
		return  DB_PAR_ERROR;
	aUsers.RemoveAll();
	aUsers.SetSize(0);
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  USERSEQUIMENT where EQUIKEY='%s'",szEquimentId);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到设备%s的用户列表信息失败.(SQL:%s)",szEquimentId,szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		while(!rst.IsEof())
		{
			szTemp.Empty();
			rst.GetFieldValue("USERID",szTemp);
	        if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft(),szTemp.TrimRight();
				aUsers.Add(szTemp);
			}			
			rst.MoveNext();	
		}	
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到设备%s的用户列表信息失败.(Description:%s, SQL:%s)",
			szEquimentId,(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
}

//得到JJA的短信息控制命令信息格式;
int CDBWork::GetMTJJACommandInfo(const TEMP_GUARDSMS &asGuardSMS,TEMP_GUARDCOMMANDSMSINFO &  asSMSCommandInfo)
{
    int iCount  =asGuardSMS.GetSize();
	if(iCount<=0)
		return DB_PAR_ERROR;
	asSMSCommandInfo.RemoveAll();
	asSMSCommandInfo.SetSize(0);
	
	for(int iLoop=0;iLoop<iCount;iLoop++)
	{
	}
	return  DB_OK;
}
//的到安放设备的控制短信息;
int CDBWork::GetGuardSMS(TEMP_GUARDSMS &aGuardSMS)
{
	aGuardSMS.RemoveAll();
	aGuardSMS.SetSize(0);
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  MTDOWNLOADSMS");		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到安防设备的控制短信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		while(!rst.IsEof())
		{
			szTemp.Empty();
			rst.GetFieldValue("ISOK",szTemp);
			szTemp.TrimLeft(),szTemp.TrimRight();
			if(szTemp=="1" || szTemp=="0")
			{
				rst.MoveNext();
				continue;
			}

			CGuardSMS  SMS;			
			rst.GetFieldValue("SIMID",szTemp);
			if(szTemp.IsEmpty())
				;
			else
			{
				szTemp.TrimLeft(),szTemp.TrimRight();
				SMS.szSIMID=szTemp;
			}
			
			rst.GetFieldValue("SMS",szTemp);
			if(szTemp.IsEmpty())
				;
			else
			{
				szTemp.TrimLeft(),szTemp.TrimRight();
				SMS.szSMS=szTemp;
			}

			rst.GetFieldValue("time",szTemp);
			if(szTemp.IsEmpty())
				;
			else
			{
				szTemp.TrimLeft(),szTemp.TrimRight();
				SMS.szTime=szTemp;
			}

			rst.GetFieldValue("REMARK",szTemp);
			SMS.szRemark=szTemp;
			aGuardSMS.Add(SMS);	
			rst.MoveNext();	
		}	
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到安防设备的控制短信息失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
	


}

//web的控制命令

int CDBWork::GetWebSMS(TEMP_WEBSMS &aWebSMS)
{
	aWebSMS.RemoveAll();
	aWebSMS.SetSize(0);
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  MTWEBCOMMANDINFO");		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到WEB的控制短信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		while(!rst.IsEof())
		{
			int  i=-1;
			rst.GetFieldValue("ISSENDSUCCESS",i);	
			if(i==1 ||i==2)
			{
				rst.MoveNext();
				continue;
			}

			szTemp.Empty();
			CWEBSMS  SMS;			
			rst.GetFieldValue("SENDSIMID",szTemp);
			if(szTemp.IsEmpty())
				;
			else
			{
				szTemp.TrimLeft(),szTemp.TrimRight();
				SMS.szSENDSIMID=szTemp;
			}
			szTemp.Empty();
			rst.GetFieldValue("MTWEBCOMMANDCONTENT",szTemp);
			if(szTemp.IsEmpty())
				;
			else
			{
				szTemp.TrimLeft(),szTemp.TrimRight();
				SMS.szMTWEBCOMMANDCONTENT=szTemp;
			}
			szTemp.Empty();
			rst.GetFieldValue("TIMELOG",szTemp);
			if(szTemp.IsEmpty())
				;
			else
			{
				szTemp.TrimLeft(),szTemp.TrimRight();
				SMS.szTIMELOG=szTemp;
			}
			i=-1;
			rst.GetFieldValue("SOCKETINDEX",i);
			SMS.SOCKETINDEX=i;
			aWebSMS.Add(SMS);	
			rst.MoveNext();	
		}	
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到WEB控制短信息失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;	
}
//end

int CDBWork::GetMTJJACommandInfo(const  CString & szToEquimentId,const CGuardSMS  & guardSMS,CGuardCommandSMSInfo  &info)
{
	CString szSimID=guardSMS.szSIMID;
	CString  szSMS=guardSMS.szSMS;
	if(szSimID.IsEmpty()  ||szSMS.IsEmpty() )
		return DB_PAR_ERROR;
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		//select * from  MTJJACOMMANDINFO   where  SIMID like '%13370270192%'
		szSQL.Format("select * from  MTCOMMANDINFO  where  SIMID like '%%%s%%'  and INFO='%s'  and TOSIMID='%s'",szSimID,szSMS,szToEquimentId);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到安防设备的控制短信息命令失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		if(!rst.IsEof())
		{
			szTemp.Empty();
			rst.GetFieldValue("CODEID",szTemp);
			if(szTemp.IsEmpty())
				return DB_EXE_ERROR;
			else
			{
				szTemp.TrimLeft(),szTemp.TrimRight();
				info.szConent=szTemp;
			}

			rst.GetFieldValue("TOSIMID",szTemp);
			if(szTemp.IsEmpty())
				return DB_EXE_ERROR;
			else
			{
				szTemp.TrimLeft(),szTemp.TrimRight();
				info.szToSimid=szTemp;
			}

			rst.GetFieldValue("INFO",szTemp);
			if(szTemp.IsEmpty())
				return DB_EXE_ERROR;
			else
			{
				szTemp.TrimLeft(),szTemp.TrimRight();
				info.szInfo=szTemp;
			}
			rst.GetFieldValue("SIMID",szTemp);
			if(szTemp.IsEmpty())
				return DB_EXE_ERROR;
			else
			{
				szTemp.TrimLeft(),szTemp.TrimRight();
				info.szSIMID=szTemp;
			}
		}
		else
			return  DB_EXE_ERROR;
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到安防设备的控制短信息命令失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
	

}
//GPS短信息控制命令

int CDBWork::GetMTGPSCommand_SMSInfo(const  CString & szToEquimentId,const CGuardSMS  & GPsSMS,CGPSCommandSMSInfo  &info)
{
	CString szSimID=GPsSMS.szSIMID;
	CString  szSMS=GPsSMS.szSMS;
	if(szSimID.IsEmpty()  ||szSMS.IsEmpty() )
		return DB_PAR_ERROR;
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		//select * from  MTJJACOMMANDINFO   where  SIMID like '%13370270192%'
		szSQL.Format("select A.*,B.*  from  MTGPSSMSCOMMANDINFO A,GPSCTRTEL B    where  B.TELS  like '%%%s%%'  and A.COMMANDTEXT='%s'  and A.TOEQUIMENTID='%s'  and A.TOEQUIMENTID=B.EUQIMENTID",szSimID,szSMS,szToEquimentId);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到监控设备的控制短信息命令失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		if(!rst.IsEof())
		{
		
		}
		else
			return  DB_EXE_ERROR;
			szTemp.Format("得到监控设备的控制短信息命令OK.(SQL:%s)",szSQL);
			g_Log(szTemp);
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到监控设备的控制短信息命令失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
	
	
}

int CDBWork::UpdateGuardSMS(const CGuardSMS &SMS,BOOL BeOK)
{
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		//select * from  MTJJACOMMANDINFO   where  SIMID like '%13370270192%'
		if(BeOK)
		{
			szTemp="1";
		}
		else
			szTemp="0";

		szSQL.Format("update MTDOWNLOADSMS  set ISOK='%s'  where  SIMID='%s' and SMS='%s'  and TIME='%s'",szTemp,SMS.szSIMID,SMS.szSMS,SMS.szTime);		
//	return  DB_OK;
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("更新安防设备的控制短信息命令失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
	}
	catch(_com_error &e)
	{
		szTemp.Format("更新安防设备的控制短信息命令失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}
//插入监控设备短信设置的信息返回临时的
int CDBWork::UpdateGPSTEMPSMS(const CGuardSMS &SMS,BOOL bSendOK)
{
	
	CString szSimID=SMS.szSIMID;
	CString  szSMS=SMS.szSMS;
	if(szSimID.IsEmpty()  ||szSMS.IsEmpty() )
		return DB_PAR_ERROR;

	
	{
		if(bSendOK)
		{
			szSMS+="发送成功";
		}
		else
			szSMS+="发送失败请检查设备,或者和供应商联系";

	}

	
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		COleDateTime  now=COleDateTime::GetCurrentTime();
		
		//added  by tgc 20070423  增加一个id的项;
        int index;
		if(GetGpsTempReturnId(index)<0)
		{
			index=0;
		}
		szSQL.Format("insert into  GPSSMSCOMMANDRETURNTEMP(FROMSIMID,RESULT,ID)values('%s','%s',%d)",szSimID,szSMS,index);		
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("插入监控设备的控制短信息返回命令失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("插入监控设备的控制短信息返回命令失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
	
}
//end
//
//插入监控设备web设置的信息返回临时的
int CDBWork::UpdateGPSTEMPWEB(const CWEBSMS &SMS,BOOL bSendOK)
{
	

	CString  szSMS;
	CString  szText=SMS.szMTWEBCOMMANDCONTENT;
	
	if(szText.IsEmpty() )
		return DB_PAR_ERROR;
	if(szText.GetLength()<15)
		return  DB_PAR_ERROR;
	CString  szEuqimentId=szText.Mid(1,11);

	if(bSendOK)
	{
			szSMS="发送成功";
	}
	else
		szSMS="发送失败请检查设备,或者和供应商联系";	
	
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		COleDateTime  now=COleDateTime::GetCurrentTime();
		
		
		szSQL.Format("insert into  GPSWEBCOMMANDRETURNTEMP(SOCKETINDEX,TEXT,EQUIMENTID)values(%d,'%s','%s')",SMS.SOCKETINDEX,szSMS,szEuqimentId);		
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("插入监控设备的WBE控制信息返回命令失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("插入监控设备的WBE控制信息返回命令失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
	
}
//end

int CDBWork::InsertGuardSMSReturn(const CGuardSMS &SMS, BOOL bSendOK)
{
	static  int inCount=0;
	if(inCount<=0)
	{
		if(GetTableMaxID("MTSENDSMS",inCount)>0)
		{

		}
		else
			inCount=0;

	}
	inCount++;

	CString szSimID=SMS.szSIMID;
	CString  szSMS=SMS.szSMS;
	if(szSimID.IsEmpty()  ||szSMS.IsEmpty() )
		return DB_PAR_ERROR;
	if(bSendOK)
	{
		szSMS+="发送成功";
	}
	else
		szSMS+="发送失败请检查设备,或者和供应商联系";

	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		COleDateTime  now=COleDateTime::GetCurrentTime();


		szSQL.Format("insert into  MTSENDSMS(SIMID,TIME,SMS,ID)values('%s','%s','%s',%d)",szSimID,now.Format("%Y-%m-%d %H:%M:%S"),szSMS,inCount);		
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("插入安防设备的控制短信息返回命令失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}

	}
	catch(_com_error &e)
	{
		szTemp.Format("插入安防设备的控制短信息返回命令失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

//web
int CDBWork::InsertWebReturn(const CWEBSMS &SMS,CString szText, BOOL bSendOK)
{
	static int inCount=0;
	CString szSMS;
	if(inCount<=0)
	{
		if(GetReturnTableMaxID("MTWEBRETURNINFO",inCount)>0)
		{
			
		}
		else
			inCount=0;
		
	}
	inCount++;
	if(bSendOK)
	{
		szSMS+="发送成功  "+szText;
	}
	else
		szSMS+="发送失败请检查设备,或者和供应商联系  "+szText;
	
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		COleDateTime  now=COleDateTime::GetCurrentTime();
		
		int  istate=0;
		szSQL.Format("insert into  MTWEBRETURNINFO(COMMAND_ID,COMMAND_STR,COMMAND_TIME,SOCKETINDEX,COMMAND_STATE,COMMAND_STAT)values(%d,'%s','%s',%d,%d,%d)",inCount,szSMS,now.Format("%Y-%m-%d %H:%M:%S"),SMS.SOCKETINDEX,istate,istate);		
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("插入设备的WEB控制短信息返回命令失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("插入设备的WEB控制短信息返回命令失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
	
}
//end

int CDBWork::GetTableMaxID(CString szTableName, int &iMaxID)
{
	if(szTableName.IsEmpty())
		return DB_PAR_ERROR;
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		//select * from  MTJJACOMMANDINFO   where  SIMID like '%13370270192%'
		szSQL.Format("select MAX(ID) as MAXID from %s",szTableName);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到%s的最大号失败.(SQL:%s)",szTableName,szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		if(!rst.IsEof())
		{
			rst.GetFieldValue("MAXID",iMaxID);
			if(iMaxID<0)
				return  DB_EXE_ERROR;
		}
		else
			return  DB_EXE_ERROR;
	}
	catch(_com_error &e)
	{
		szTemp.Format("得得到%s的最大号失败.(Description:%s, SQL:%s)",szTableName,szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}
//
int CDBWork::GetReturnTableMaxID(CString szTableName, int &iMaxID)
{
	if(szTableName.IsEmpty())
		return DB_PAR_ERROR;
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		//select * from  MTJJACOMMANDINFO   where  SIMID like '%13370270192%'
		szSQL.Format("select MAX(COMMAND_ID) as MAXID from %s",szTableName);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到%s的最大号失败.(SQL:%s)",szTableName,szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		if(!rst.IsEof())
		{
			rst.GetFieldValue("MAXID",iMaxID);
			if(iMaxID<0)
				return  DB_EXE_ERROR;
		}
		else
			return  DB_EXE_ERROR;
	}
	catch(_com_error &e)
	{
		szTemp.Format("得得到%s的最大号失败.(Description:%s, SQL:%s)",szTableName,szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
	
}

//更新web命令表
int CDBWork::UpdateWebSMS(const CWEBSMS &SMS,BOOL BeOK)
{
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	int   IsSuccess=-1;
	try
	{
		//select * from  MTJJACOMMANDINFO   where  SIMID like '%13370270192%'
		if(BeOK)
		{
			IsSuccess=1;
		}
		else
			IsSuccess=2;
		
		szSQL.Format("update MTWEBCOMMANDINFO  set ISSENDSUCCESS=%d  where  MTWEBCOMMANDCONTENT='%s' and SOCKETINDEX=%d  and TIMELOG='%s'",IsSuccess,SMS.szMTWEBCOMMANDCONTENT,SMS.SOCKETINDEX,SMS.szTIMELOG);		
		//	return  DB_OK;
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("更新web的控制短信息命令失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
	}
	catch(_com_error &e)
	{
		szTemp.Format("更新web的控制短信息命令失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
	
}

int CDBWork::GetGpsData(CString szEuqimentId, CString szBeginTime, CString szEndTime, int Type, TEMP_GPSDATA &asGpsData)
{
	if(szEuqimentId.IsEmpty()  ||szBeginTime.IsEmpty()  ||szEndTime.IsEmpty())
	{
		return  DB_PAR_ERROR;
	}
	if(Type<0 ||Type>2)
		return  DB_PAR_ERROR;
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	asGpsData.RemoveAll();
	asGpsData.SetSize(0);


	try
	{
	//	 0:全部：1速度不为0的点，2有状态回应的点;
		//处理时间段addede 20070707 
	//	CString  szEndDate="2007-07-12 20:00:00";
		CStringArray   szSQLArray;
		szSQLArray.RemoveAll();
		szSQLArray.SetSize(0);
		COleDateTime OleYTime(2007,8,15,15,20,0);
		COleDateTime  oleBeginTime,oleEndTime;
		COleDateTimeSpan   Span(1,0,0,0);
        oleBeginTime.ParseDateTime(szBeginTime);
        oleEndTime.ParseDateTime(szEndTime);
		if(oleBeginTime.GetStatus()!=COleDateTime::valid || oleEndTime.GetStatus()!=COleDateTime::valid )
			return  DB_PAR_ERROR;
		if(oleEndTime<=OleYTime)
		{
			//数据存在以前的数据库里;
			if(Type==0)
			{
				szSQL.Format("select  *  from  GPSDATA  where  EQUIMENTID='%s' and DATATIME<='%s' and DATATIME>='%s'",szEuqimentId,szEndTime,szBeginTime);
			}
			if(Type==1)
			{
				szSQL.Format("select  *  from  GPSDATA  where  EQUIMENTID='%s' and DATATIME<='%s' and DATATIME>='%s' and  SPEED>=0.0001",szEuqimentId,szEndTime,szBeginTime);
			}
			if(Type==2)
			{
				szSQL.Format("select  *  from  GPSDATA  where  EQUIMENTID='%s' and DATATIME<='%s' and DATATIME>='%s' and  STATE>0",szEuqimentId,szEndTime,szBeginTime);
				
			}
			szSQLArray.Add(szSQL);
		}
		else
		{
			

			if(oleBeginTime<OleYTime)
			{
				if(Type==0)
				{
					szSQL.Format("select  *  from  GPSDATA  where  EQUIMENTID='%s' and DATATIME<='%s' and DATATIME>='%s'",szEuqimentId,szEndTime,szBeginTime);
				}
				if(Type==1)
				{
					szSQL.Format("select  *  from  GPSDATA  where  EQUIMENTID='%s' and DATATIME<='%s' and DATATIME>='%s' and  SPEED>=0.0001",szEuqimentId,szEndTime,szBeginTime);
				}
				if(Type==2)
				{
					szSQL.Format("select  *  from  GPSDATA  where  EQUIMENTID='%s' and DATATIME<='%s' and DATATIME>='%s' and  STATE>0",szEuqimentId,szEndTime,szBeginTime);
					
				}
				szSQLArray.Add(szSQL);
				oleBeginTime=OleYTime;
			}
			oleBeginTime.SetDateTime(oleBeginTime.GetYear(),oleBeginTime.GetMonth(),oleBeginTime.GetDay(),0,0,0);
			oleEndTime.SetDateTime(oleEndTime.GetYear(),oleEndTime.GetMonth(),oleEndTime.GetDay(),0,0,0);
			
			while (oleBeginTime<=oleEndTime) 
			{
				CString  szTableName;
				szTableName.Format("GPSDATA%s",oleBeginTime.Format("%Y%m%d"));
				if(Type==0)
				{
					szSQL.Format("select  *  from  %s  where  EQUIMENTID='%s' and DATATIME<='%s' and DATATIME>='%s'",szTableName,szEuqimentId,szEndTime,szBeginTime);
				}
				if(Type==1)
				{
					szSQL.Format("select  *  from  %s  where  EQUIMENTID='%s' and DATATIME<='%s' and DATATIME>='%s' and  SPEED>=0.0001",szTableName,szEuqimentId,szEndTime,szBeginTime);
				}
				if(Type==2)
				{
					szSQL.Format("select  *  from  %s  where  EQUIMENTID='%s' and DATATIME<='%s' and DATATIME>='%s' and  STATE>0",szTableName,szEuqimentId,szEndTime,szBeginTime);
					
				}
				szSQLArray.Add(szSQL);
				oleBeginTime+=Span;
			}
			
			

		}
		//end;
		int  iCOunt=szSQLArray.GetSize();
		for(int  iLoop=0;iLoop<iCOunt;iLoop++)
		{
			szSQL=szSQLArray.GetAt(iLoop);
			if(!m_Database.Execute(szSQL, rst))
			{
				szTemp.Format("得到%s的gps数据失败.(SQL:%s)",szEuqimentId,szSQL);
				g_Log(szTemp);
				continue;
			}
			
			while(!rst.IsEof())
			{
				CString szTemp;
				CGPSData  gpsdata;
				rst.GetFieldValue("EQUIMENTID",szTemp);
				if(szTemp.IsEmpty())
				{
					rst.MoveNext();
					continue;
				}
				sprintf(gpsdata.szEquimentID,"%s",szTemp);
				double  fTemp;
				rst.GetFieldValue("LONGITUDE",fTemp);
				if(fTemp<=0)
				{
					rst.MoveNext();
					continue;
				}
				gpsdata.fLongitude=fTemp;
				
				fTemp=-1;
				rst.GetFieldValue("LATITUDE",fTemp);
				if(fTemp<=0)
				{
					rst.MoveNext();
					continue;
				}
				gpsdata.fLatitude=fTemp;
				int  itemp=-1;
				rst.GetFieldValue("DIRECTION",itemp);
				if(itemp<0)
				{
					rst.MoveNext();
					continue;
				}
				gpsdata.iDirection=itemp;
				
				itemp=-1;
				rst.GetFieldValue("STATE",itemp);
				if(itemp<0)
				{
					rst.MoveNext();
					continue;
				}
				gpsdata.iState=itemp;
				
				szTemp.Empty();
				rst.GetFieldValue("DATATIME",szTemp);
				if(szTemp.IsEmpty())
				{
					rst.MoveNext();
					continue;
				}
				sprintf(gpsdata.Time,"%s",szTemp);
				
				fTemp=-1;
				rst.GetFieldValue("SPEED",fTemp);
				if(fTemp<0)
				{
					rst.MoveNext();
					continue;
				}
				gpsdata.fSpeed=fTemp;
				int  iflag;
				rst.GetFieldValue("FLAG",iflag);
				if(iflag<0)
				{
					rst.MoveNext();
					continue;
				}
				gpsdata.iFlag=iflag;
				asGpsData.Add(gpsdata);
				rst.MoveNext();       
			}
			
		}
	
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到%s的gps数据失败.(Description:%s, SQL:%s)",szEuqimentId,szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

//得到GPS设备的报警处理;
int CDBWork::GetGpsAlarmDealWithInfo(CObArray  &asGpsAlarmDealWithInfo,BOOL  bAll)
{
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	int iCount=asGpsAlarmDealWithInfo.GetSize();
	for(int index=0;index<iCount;index++)
	{
		CGpsAlarmDealWithInfo   *GpsAlarminfo=NULL;
		GpsAlarminfo=(CGpsAlarmDealWithInfo *)asGpsAlarmDealWithInfo.GetAt(index);
		if(GpsAlarminfo)
		{
			delete  GpsAlarminfo;
			GpsAlarminfo=NULL;
		}

	}
	asGpsAlarmDealWithInfo.RemoveAll();
	asGpsAlarmDealWithInfo.SetSize(0);
	try
	{

        if(bAll)
		{
			//取得所有的信息
			szSQL.Format("select  A.*,B.CONTACTTEL  from  GPSALARMDESCRIPTION A,TELSMSCONTACT B where  A.EQUIMENTID=B.EQUIMENTID  ");
		}
		else
		{
			//只读取需要发送的数据;  //在服务器程序上使用
			szSQL.Format("select  A.*,B.CONTACTTEL  from  GPSALARMDESCRIPTION A,TELSMSCONTACT B where  A.EQUIMENTID=B.EQUIMENTID   and  A.BESEND=1 ");
		}

		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到Gps设备的报警处理信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		while(!rst.IsEof())
		{
			CString szTemp;
			CGpsAlarmDealWithInfo   *GpsAlarminfo=NULL;
			GpsAlarminfo=  new   CGpsAlarmDealWithInfo;
			if(!GpsAlarminfo)
			{
				rst.MoveNext();
				continue;
			}  //maybe  error;
			rst.GetFieldValue("EQUIMENTID",szTemp);
			if(szTemp.IsEmpty())
			{
				rst.MoveNext();
				continue;
			}
			sprintf(GpsAlarminfo->EQUIMENTID,"%s",szTemp);

			int   iTemp;
			rst.GetFieldValue("STATE",iTemp);
			if(iTemp<0)
			{
				rst.MoveNext();
				continue;
			}
			GpsAlarminfo->STATE=iTemp;
			
            iTemp=-1;
			rst.GetFieldValue("GRADE",iTemp);
			if(iTemp<=0)
			{
				rst.MoveNext();
				continue;
			}
			GpsAlarminfo->GRADE=iTemp;
			
			iTemp=-1;
			rst.GetFieldValue("ALARMNUMBER",iTemp);
			if(iTemp<0)
			{
				rst.MoveNext();
				continue;
			}
			GpsAlarminfo->ALARMNUMBER=iTemp;
			
			iTemp=-1;
			rst.GetFieldValue("BESEND",iTemp);
			if(iTemp<0)
			{
				rst.MoveNext();
				continue;
			}
			GpsAlarminfo->BESEND=iTemp;
			
			
			szTemp.Empty();
			rst.GetFieldValue("DEALWITH",szTemp);
			if(szTemp.IsEmpty())
			{
				rst.MoveNext();
				continue;
			}
			sprintf(GpsAlarminfo->DEALWITH,"%s",szTemp);

			szTemp.Empty();
			rst.GetFieldValue("DESCRIPTION",szTemp);
			if(szTemp.IsEmpty())
			{
				rst.MoveNext();
				continue;
			}
			sprintf(GpsAlarminfo->DESCRIPTION ,"%s",szTemp);

			
			szTemp.Empty();
			rst.GetFieldValue("CONTACTTEL",szTemp);
			if(szTemp.IsEmpty())
			{
				rst.MoveNext();
				continue;
			}
			sprintf(GpsAlarminfo->TELS ,"%s",szTemp);
			

			asGpsAlarmDealWithInfo.Add((CObject  *)GpsAlarminfo);
			rst.MoveNext();       
		}
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到Gps设备的报警处理信息失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

int CDBWork::GetEquimentIdFormNumber(CString szNum, CString &szEuqimentId)
{
	szNum.TrimLeft(),szNum.TrimRight();
	if(szNum.IsEmpty() )
		return   DB_PAR_ERROR;

	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	
	try
	{    
		szSQL.Format("select  *  from  EQUIMENTINFO   where  EQUIMENTNUMBER='%s'",szNum);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到%s设备的ID信息失败.(SQL:%s)",szNum,szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}		
		if(!rst.IsEof())
		{
			CString szTemp;
			rst.GetFieldValue("EQUIMENTID",szTemp);
			szTemp.TrimLeft(),szTemp.TrimRight();
			if(szTemp.IsEmpty())
				return  DB_EXE_ERROR;
			else
			   szEuqimentId=szTemp;
		
		}
		else
			return  DB_EXE_ERROR;

		
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到设备的ID信息失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

int CDBWork::UpdateUserPws(CString szUserId, CString szNewPws)
{
	szUserId.TrimRight(),szUserId.TrimLeft();
	szNewPws.TrimRight(),szNewPws.TrimLeft();
	if(szNewPws.IsEmpty() || szUserId.IsEmpty())
		return  DB_PAR_ERROR;
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{	
		
		szSQL.Format("update USERIINFO  set PASSWORD='%s'  where  USERID='%s'",szNewPws,szUserId);
		//	return  DB_OK;
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("更新用户密码失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
	}
	catch(_com_error &e)
	{
		szTemp.Format("更新用户密码失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

BOOL CDBWork::IsUserValiDate(CString szUserId, CString szValiDate)
{
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  USERVALIDATE  where USERID='%s' and PWS='%s'",szUserId,szValiDate);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("验证用户验证号合法性失败.(SQL:%s)", szSQL);
			g_Log(szTemp);
			return FALSE;
		}
		if(!rst.IsEof())
		{
			rst.Close();
			return  TRUE;
		}
		else
			return FALSE;
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("验证用户验证号合法性失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return FALSE;
	}	
	return TRUE;

}

int CDBWork::UpdateUserValiDate(CString szUserId, CString szValiDate)
{
	szUserId.TrimRight(),szUserId.TrimLeft();
	szValiDate.TrimRight(),szValiDate.TrimLeft();
	if(szValiDate.IsEmpty() || szUserId.IsEmpty())
		return  DB_PAR_ERROR;
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{	
		
		szSQL.Format("update USERVALIDATE  set PWS='%s'  where  USERID='%s'",szValiDate,szUserId);
		//	return  DB_OK;
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("更新用户验证号失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
	}
	catch(_com_error &e)
	{
		szTemp.Format("更新用户验证号失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

BOOL CDBWork::IsUserSEquiment(const CString &szUserId, const CString &szEquimentId)
{
	if(szUserId.IsEmpty() || szEquimentId.IsEmpty() )
		return  FALSE;
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  USERSEQUIMENT  where USERID='%s' and EQUIKEY='%s'",szUserId,szEquimentId);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("验证用户设备合法性失败.(SQL:%s)", szSQL);
			g_Log(szTemp);
			return FALSE;
		}
		if(!rst.IsEof())
		{
			rst.Close();
			return  TRUE;
		}
		else
			return FALSE;
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("验证用户设备合法性失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return FALSE;
	}	
	return TRUE;

}
int CDBWork::GetUserPower(CString szUserId, char *pbuf, int iLen)
{
    szUserId.TrimLeft(),szUserId.TrimRight();
	if(szUserId.IsEmpty() || pbuf==NULL  ||iLen>300)
		return  DB_PAR_ERROR;
	memset(pbuf,0,iLen);
	CString  szSql,szTemp;
	szSql.Format("select POWER from USERPOWER  where  USERID='%s'",szUserId);
	
	CADORecordset rst(&m_Database);
	try
	{
		if(!m_Database.Execute(szSql, rst))
		{
			szTemp.Format("得到用户权限信息失败.(SQL:%s)",szSql);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		if (!rst.IsEof())
		{
		
			
			CString  szInfoValue;
			rst.GetFieldValue("POWER",szInfoValue);
			int istrlen=szInfoValue.GetLength();
			//	int len=istrlen>300  ?   iLen: istrlen;
			sprintf(pbuf,"%s",szInfoValue);
			
			
		}
		else
			DB_EXE_ERROR;
		
		
	}
	catch (_com_error e) 
	{
		szTemp.Format("得到用户权限信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSql);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
	
	
}

int CDBWork::InsertOpaterResult(COpeateResult *pDate)
{

	if(!pDate)
		return DB_PAR_ERROR;
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{

		
		int  istate=0;
		szSQL.Format("insert into  OPERATERESULT(USERID,EUQIMENTID,TIME,RESULT,ALARMINFO)"
			"  values('%s','%s','%s','%s','%s')",pDate->USERID,pDate->EUQIMENTID,pDate->TIME,pDate->RESULT,pDate->ALARMINFO);
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("插入处理结果失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("插入处理结果失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

int CDBWork::InsertTelMessage(CMessageToClient *pDate)
{
	if(!pDate)
		return DB_PAR_ERROR;
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		
		
		COleDateTime  now=COleDateTime::GetCurrentTime();
		szSQL.Format("insert into  MTDOWNLOADSMS(SIMID,SMS,TIME)"
			"  values('%s','%s','%s')",pDate->szTel,pDate->Message,now.Format("%Y-%m-%d %H:%M:%S"));
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("插入手机信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("插入手机信息失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

int CDBWork::GetCtrTels(CString szEuqimentId, CCtrTels &CtrTels)
{
	if(szEuqimentId.IsEmpty())
		return  DB_PAR_ERROR;
	CString szSQL, szTemp;

	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  GPSCTRTEL  where EUQIMENTID='%s'",szEuqimentId);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到设备控制手机信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		if(!rst.IsEof())
		{
			sprintf(CtrTels.EUQIMENTID,"%s",szEuqimentId);
			rst.GetFieldValue("TELS",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(CtrTels.TELS,"%s",szTemp);
				CtrTels.TELS[szTemp.GetLength()]=0;
			}
			rst.Close();
		}
		else
			return DB_EXE_ERROR;
		
		
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到设备控制手机信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;


}

int CDBWork::GetAlarmRecvTels(CString szEquimentId, CAlarmRecvTels &info)
{
	if(szEquimentId.IsEmpty())
		return  DB_PAR_ERROR;
	CString szSQL, szTemp;
	
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  TELSMSCONTACT  where EQUIMENTID='%s'",szEquimentId);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到设备报警接受手机信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		if(!rst.IsEof())
		{
			sprintf(info.EQUIMENTID,"%s",szEquimentId);
			rst.GetFieldValue("CONTACTTEL",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(info.CONTACTTEL,"%s",szTemp);
				info.CONTACTTEL[szTemp.GetLength()]=0;
			}
			rst.Close();
		}
		else
			return DB_EXE_ERROR;
		
		
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到设备报警接受手机信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

int CDBWork::GetFeeInfo(CString szEquimentId, CFeeInfo &info)
{

	if(szEquimentId.IsEmpty())
		return  DB_PAR_ERROR;
	CString szSQL, szTemp;
	
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  FEEINFO  where EQUIMENTID='%s'",szEquimentId);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到设备费用信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		if(!rst.IsEof())
		{
			sprintf(info.EQUIMENTID,"%s",szEquimentId);
			
			rst.GetFieldValue("SERVERTYPE",szTemp);
			sprintf(info.SERVERTYPE,"%s",szTemp);
			info.SERVERTYPE[szTemp.GetLength()]=0;

			rst.GetFieldValue("OPENTIME",szTemp);
			sprintf(info.OPENTIME,"%s",szTemp);
			info.OPENTIME[szTemp.GetLength()]=0;
			
			rst.GetFieldValue("FEETIME",szTemp);
			sprintf(info.FEETIME,"%s",szTemp);
			info.FEETIME[szTemp.GetLength()]=0;
             
			int  i=-1;
			rst.GetFieldValue("FEE",i);
			info.FEE=i;

			rst.GetFieldValue("STRATTIME",szTemp);
			sprintf(info.STRATTIME,"%s",szTemp);
			info.STRATTIME[szTemp.GetLength()]=0;

			rst.GetFieldValue("ENDTIME",szTemp);
			sprintf(info.ENDTIME,"%s",szTemp);
			info.ENDTIME[szTemp.GetLength()]=0;
	        //COMPANYNO
			rst.GetFieldValue("COMPANYNO",szTemp);
			sprintf(info.CompanyNo,"%s",szTemp);
			info.CompanyNo[szTemp.GetLength()]=0;
			
		//	rst.Close();
		}
		else
			return DB_EXE_ERROR;
	/*
		//得到公司编号;
			CString szCompanyNO;
			if(GetCompanyNoFormEuqimentId(szEquimentId,szCompanyNO)<0)
			{
				szCompanyNO="000000";
	
			}
			sprintf(info.CompanyNo,"%s",szCompanyNO);
	*/
	

		
		
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到设备费用信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
}

int CDBWork::GetGpsAlarmInfo(CString szEquimentId, TEMP_GpsAlarmInfo &aGpsAlarmInfo)
{
	if(szEquimentId.IsEmpty())
		return  DB_PAR_ERROR;
	aGpsAlarmInfo.RemoveAll();
	aGpsAlarmInfo.SetSize(0);
	CString szSQL, szTemp;
	
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  GPSALARMDESCRIPTION  where EQUIMENTID='%s'",szEquimentId);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到设备报警描述信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		while(!rst.IsEof())
		{
			CGpsAlarmInfo   info;
			sprintf(info.EuqimentId,"%s",szEquimentId);
			
			rst.GetFieldValue("DESCRIPTION",szTemp);
			sprintf(info.info,"%s",szTemp);
			info.info[szTemp.GetLength()]=0;
			
			rst.GetFieldValue("DEALWITH",szTemp);
			sprintf(info.DealWith,"%s",szTemp);
			info.DealWith[szTemp.GetLength()]=0;

			
			int  i=-1;
			rst.GetFieldValue("STATE",i);
			info.AlarmState=i;
			
			i=-1;
			rst.GetFieldValue("GRADE",i);
			info.Grade=i;
			
			i=-1;
			rst.GetFieldValue("BESEND",i);
			info.BESEND=i;

			i=-1;
			rst.GetFieldValue("ALARMNUMBER",i);
			info.AlarmNumber=i;


			aGpsAlarmInfo.Add(info);
			rst.MoveNext();			
		}
		rst.Close();	
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到设备报警描述信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

int CDBWork::GetCompanyNoFormEuqimentId(CString szEquimentId, CString &CompanyNo)
{

	szEquimentId.TrimLeft(),szEquimentId.TrimRight();
	if(szEquimentId.IsEmpty())
		return  DB_PAR_ERROR;
	CString szSQL, szTemp;
	CString szUserId;
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  USERSEQUIMENT  where EQUIKEY='%s'",szEquimentId);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到设备的公司编号信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		if(!rst.IsEof())
		{		
			rst.GetFieldValue("USERID",szTemp);
			szUserId=szTemp;
			szUserId.TrimLeft(),szUserId.TrimRight();
			if(szUserId.IsEmpty())
			{
				return DB_EXE_ERROR;
				
			}
		}
		else
			return DB_EXE_ERROR;

		//得到公司编号;
		szSQL.Format("select B.COMPANYNO as COMPANYNO   from  USERIINFO A,COMPANYINFO B"
			"  where A.PARENTCOMPANY=B.COMPANYNAME  and A.USERID='%s'",szUserId);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到设备的公司编号信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		if(!rst.IsEof())
		{
			rst.GetFieldValue("COMPANYNO",szTemp);
			CompanyNo=szTemp;

		}
		else
			return  DB_EXE_ERROR;
		

			
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到设备的公司编号信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

// 更新报警接收手机
int CDBWork::UpdateAlarmRecvTels(CString szEquimentId, CString szTels)
{
	szEquimentId.TrimLeft(),szEquimentId.TrimRight();
	szTels.TrimLeft(),szTels.TrimRight();
	if(szEquimentId.IsEmpty() ||szEquimentId.IsEmpty())
		return DB_PAR_ERROR;
	if(DeleteAlarmRecvTels(szEquimentId)<0)
		return  DB_EXE_ERROR;
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("insert into  TELSMSCONTACT(EQUIMENTID,CONTACTTEL)"
			"  values('%s','%s')",szEquimentId,szTels);
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("插入报警接收手机失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("插入报警接收手机失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

//删除报警接收手机
int CDBWork::DeleteAlarmRecvTels(CString szEquimentId)
{
	szEquimentId.TrimLeft(),szEquimentId.TrimRight();
	if(szEquimentId.IsEmpty())
		return DB_PAR_ERROR;
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		
		
	
		szSQL.Format("delete  from   TELSMSCONTACT  where EQUIMENTID='%s'",szEquimentId);
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("删除报警接收手机失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("删除报警接收手机失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

//删除控制手机信息
int CDBWork::DeleteCtrTels(CString szEquimentId)
{
	szEquimentId.TrimLeft(),szEquimentId.TrimRight();
	if(szEquimentId.IsEmpty())
		return DB_PAR_ERROR;
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("delete  from   GPSCTRTEL  where EUQIMENTID='%s'",szEquimentId);
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("删除控制手机信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("删除控制手机信息失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
	
}
//更新控制手机信息

int CDBWork::UpdateCtrTels(CString szEquimentId, CString szTels)
{
	szEquimentId.TrimLeft(),szEquimentId.TrimRight();
	szTels.TrimLeft(),szTels.TrimRight();
	if(szEquimentId.IsEmpty() ||szEquimentId.IsEmpty())
		return DB_PAR_ERROR;
	if(DeleteCtrTels(szEquimentId)<0)
		return  DB_EXE_ERROR;
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("insert into  GPSCTRTEL(EUQIMENTID,TELS)"
			"  values('%s','%s')",szEquimentId,szTels);
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("插入控制手机失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("插入控制手机失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
	
}

int CDBWork::UpDateGpsAlarmInfo(CGpsAlarmInfo *pInfo)
{
   if(!pInfo)
	   return DB_PAR_ERROR;
   
   CString szSQL, szTemp;
   CADORecordset rst(&m_Database);
   try
   {
	   szSQL.Format("update  GPSALARMDESCRIPTION  set DESCRIPTION='%s'"
		   "  where EQUIMENTID='%s' and STATE=%d",pInfo->info,pInfo->EuqimentId,pInfo->AlarmState);
	   if(!m_Database.Execute(szSQL))
	   {
		   szTemp.Format("更新GPS设备的报警信息失败.(SQL:%s)",szSQL);
		   g_Log(szTemp);
		   return DB_EXE_ERROR;
	   }
	   
   }
   catch(_com_error &e)
   {
	   szTemp.Format("更新GPS设备的报警信息失败.(Description:%s, SQL:%s)",szSQL);
	   g_Log(szTemp);
	   return DB_EXE_ERROR;
   }
   return  DB_OK;
   
   

}

//得到用户可用地图的里表
int CDBWork::GetUserMap(CString szUserId, CStringArray &asUserMap)
{
	szUserId.TrimLeft(),szUserId.TrimRight();
	if(szUserId.IsEmpty())
		return  DB_PAR_ERROR;
	asUserMap.RemoveAll();
	asUserMap.SetSize(0);

	CString szSQL, szTemp;

	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  USERMAP  where USERID='%s'",szUserId);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到用户的地图信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}		
	
		while(!rst.IsEof())
		{
			rst.GetFieldValue("MAP",szTemp);
			szTemp.TrimLeft(),szTemp.TrimRight();
			if(szTemp.IsEmpty())
			{
				rst.MoveNext();
				continue;
			}
			asUserMap.Add(szTemp);
			rst.MoveNext();
			
		}

		
		
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到用户的地图信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

//得到gps控制返回命令的中间表的可用id;
int CDBWork::GetGpsTempReturnId(int &index)
{
	CString szSQL, szTemp;	
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select  MAX(ID)  as ID   from  GPSSMSCOMMANDRETURNTEMP ");		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到设备控制返回信息表最大id信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		if(!rst.IsEof())
		{
			//有最大的id;
			rst.GetFieldValue("ID",index);
			index+=1;
		
		}
		else
		{
			//无最大的ID;
			index=0;  //无最大的ID的时候就从0开始 ;
		}
		rst.Close();
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到设备控制返回信息表最大id信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;


}


//得到软件的版本信息;
int CDBWork::GetSoVersion(CString szCompanyId, CString &szComPanyName, int &iVerSion)
{
	CString szSQL, szTemp;	
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select  VERSION,COMPANYNAME from  SOFTVERSION");		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到软件版本信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		if(!rst.IsEof())
		{
			//有最大的id;
			rst.GetFieldValue("COMPANYNAME",szComPanyName);
			rst.GetFieldValue("VERSION",iVerSion);
	
			
		}
		else
		{
			szComPanyName="ZT";
			iVerSion=1;


		}
		rst.Close();
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到软件版本信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
}

//得到最近的服务器的ip地址
int CDBWork::GetNewestIP(CString &Ip)
{
	Ip.Empty();
	CString szSQL, szTemp;	
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select  * from  NEWIP");		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到最近的IP信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		if(!rst.IsEof())
		{
			//有最大的id;
			rst.GetFieldValue("IP",Ip);			
		}
		rst.Close();
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到最近的IP信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

//更新最新的ip
int CDBWork::UpDateNewestIp(CString szWeb, CString szTime, CString szIP)
{

//	szTime.TrimRight(),szTime.TrimLeft();
	szIP.TrimLeft(),szTime.TrimRight();
	if(/*szTime.IsEmpty() ||*/ szIP.IsEmpty())
		return  DB_PAR_ERROR;	
	CString szSQL, szTemp;

	try
	{
		szSQL.Empty();
		szSQL.Format("delete  from NEWIP");
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("更新服务器的最新IP失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		COleDateTime  now=COleDateTime::GetCurrentTime();		
		szSQL.Format("insert into  NEWIP(IP,TIME)values('%s','%s')",szIP,now.Format("%Y-%m-%d %H:%M:%S"));		
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("更新服务器的最新IP失败..(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("更新服务器的最新IP失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

//插入用户操作纪录
int CDBWork::InsertUserevent(CString szUserID, CString szText, CString szEquimentId,CString szParam,CString UserIP,int iSOK)
{

	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{		
		COleDateTime  now=COleDateTime::GetCurrentTime();
		szSQL.Format("insert into  USEREVENTRECORD(USERID,EQUIMENTID,TEXT,TIME,PARAM,IP,BOK)"
			"  values('%s','%s','%s','%s','%s','%s',%d)",szUserID,szEquimentId,szText,now.Format("%Y-%m-%d %H:%M:%S"),szParam,UserIP,iSOK);
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("插入用户操作纪录失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("插入用户操作纪录失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

//查询gps最新数据;
//DEL void CDBWork::DoGpsDateQuery(CGpsDataQuery *pDate)
//DEL {
//DEL 
//DEL }


int CDBWork::GetGpsNewestTime(CString szEquientId, CGPSData &data)
{
	if(szEquientId.IsEmpty())
	{
		return  DB_PAR_ERROR;
	}
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
	//	 0:全部：1速度不为0的点，2有状态回应的点;
  //      if(Type==0)
	//	szSQL.Format("SELECT A.*"
	//		"	 FROM GPSDATA   a, (SELECT EQUIMENTID, Max(DATATIME) as newtime FROM GPSDATA  GROUP BY EQUIMENTID)   b"
	//		"	WHERE a.EQUIMENTID='%s' and b.EQUIMENTID=a.EQUIMENTID  and a.DataTime=b.newtime",szEquientId);
		
		szSQL.Format("select  * from GPSDATANEWEST  where  EQUIMENTID='%s'",szEquientId);
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到gps数据失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		if(!rst.IsEof())
		{
			CString szTemp;
			rst.GetFieldValue("EQUIMENTID",szTemp);	
			sprintf(data.szEquimentID,"%s",szTemp);
			double  fTemp;
			rst.GetFieldValue("LONGITUDE",fTemp);
	
			data.fLongitude=fTemp;

            fTemp=-1;
			rst.GetFieldValue("LATITUDE",fTemp);
	
			data.fLatitude=fTemp;
			int  itemp=-1;
			rst.GetFieldValue("DIRECTION",itemp);
	
			data.iDirection=itemp;

			itemp=-1;
			rst.GetFieldValue("STATE",itemp);
	
			data.iState=itemp;

			szTemp.Empty();
			rst.GetFieldValue("DATATIME",szTemp);
	
			sprintf(data.Time,"%s",szTemp);

			fTemp=-1;
			rst.GetFieldValue("SPEED",fTemp);
	
			data.fSpeed=fTemp;
			int  iflag;
			rst.GetFieldValue("FLAG",iflag);

			data.iFlag=iflag;    
		}
		else
			return DB_EXE_ERROR;

	}
	catch(_com_error &e)
	{
		szTemp.Format("得到gps数据失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}
int CDBWork::CreateTable(CString szTableName)
{
	szTableName.TrimLeft(),szTableName.TrimRight();
	if(szTableName.IsEmpty())
		return DB_EXE_ERROR;
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	szSQL.Format("select * from  %s  where  EQUIMENTID='0' ",szTableName);
	try
	{
		if(!m_Database.Execute(szSQL, rst))
		{
			//没有表就创建新的表;
			szSQL.Format("CREATE TABLE %s  ("
				" EQUIMENTID                VARCHAR(15)    NOT NULL,"
				"  DATATIME                 VARCHAR(20),"
				"  FLAG              INT,			"
				"LONGITUDE           FLOAT,	"
				" LATITUDE             FLOAT,	"
				"SPEED                FLOAT,	"
				" DIRECTION            INT,		"
				" STATE           INT)			",
				
				
				szTableName);
			g_Log(szSQL);
			m_Database.Execute(szSQL);
			
		}
	}
	catch (...) {
		return  DB_EXE_ERROR;
	}	
	return  DB_OK;
}


//added 20070712  修正速度慢的缺点;
int CDBWork::UpdataGPSNewestData(const CGPSData &pData)
{	
	
	CString   szSql,szTemp;
	CADORecordset rst(&m_Database);
	BOOL  have=FALSE;
	IsHaveGPSDataNewest(pData.szEquimentID,have);

	
	if(have)
	{
		szSql.Format("update  GPSDATANEWEST  set DataTime='%s',longitude=%f,latitude=%f,SPEED=%f,Direction=%d,State=%d,FLAG=%d"
			"   where  EquimentID='%s'  and  DataTime<'%s' ",
			pData.Time,pData.fLongitude,pData.fLatitude,pData.fSpeed,pData.iDirection,pData.iState,pData.iFlag,pData.szEquimentID,pData.Time);
		
	}
	else
	{
		szSql.Format("insert into GPSDATANEWEST(EquimentID,DataTime,longitude,latitude,SPEED,Direction,State,FLAG)"
			"  values('%s','%s',%f,%f,%f,%d,%d,%d)",
			pData.szEquimentID,pData.Time,pData.fLongitude,pData.fLatitude,pData.fSpeed,pData.iDirection,pData.iState,pData.iFlag);
		
	}
	try
	{
	//	szSql.Format("select  * from  GPSDATANEWEST ");
		if(!m_Database.Execute(szSql, rst))
		{
			szTemp.Format("写入GPSDATANEWEST表失败信息失败.(SQL:%s)",szSql);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch (_com_error  &e)
	{
		szTemp.Format("写入GPSDATANEWEST表失败信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSql);
		g_Log(szTemp);
		return DB_EXE_ERROR;
		
	}
	return DB_OK;
}

int CDBWork::IsHaveGPSDataNewest(CString szEuqimentId, BOOL &bHave)
{
	
	CString   szSql,szTemp;
	CADORecordset rst(&m_Database);
    szSql.Format("select  *  from GPSDATANEWEST  where EQUIMENTID='%s'",szEuqimentId);
	try
	{
		if(!m_Database.Execute(szSql, rst))
		{
			
			szTemp.Format("读取GPSDATANEWEST表失败信息失败.(SQL:%s)",szSql);
			g_Log(szTemp);
			bHave=FALSE;
			return DB_EXE_ERROR;
			
		}
		if(rst.IsEof())
		{//没有
	       bHave=FALSE;
			
			
		}
		else
		{
			bHave=TRUE;
		}		
		rst.Close();
		
	}
	catch(_com_error  &e)
	{
		szTemp.Format("读取GPSDATANEWEST表失败信息失败.(SQL:%s)",szSql);
		g_Log(szTemp);
		bHave=FALSE;
		return DB_EXE_ERROR;
		
	}
	return DB_OK;

}
//得到用户的最新gPS数据  
int CDBWork::GetUserGpsNewestTime(CString szUSerID, CObArray  &aGpsDate)
{
	if(szUSerID.IsEmpty())
	{
		return  DB_PAR_ERROR;
	}
	aGpsDate.RemoveAll();
	aGpsDate.SetSize(0);
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{

		szSQL.Format("select A.*  from GPSDATANEWEST A,(select  * from USERSEQUIMENT where  USERID='%s')  B where  A.EQUIMENTID=B.EQUIKEY",szUSerID);
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到gps数据失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		while(!rst.IsEof())
		{
			CGPSData  *pDate=NULL;
            pDate=new CGPSData;
			if(!pDate)
				continue;
			CString szTemp;
			rst.GetFieldValue("EQUIMENTID",szTemp);	
			sprintf(pDate->szEquimentID,"%s",szTemp);
			double  fTemp;
			rst.GetFieldValue("LONGITUDE",fTemp);
			
			pDate->fLongitude=fTemp;
			
            fTemp=-1;
			rst.GetFieldValue("LATITUDE",fTemp);
			
			pDate->fLatitude=fTemp;
			int  itemp=-1;
			rst.GetFieldValue("DIRECTION",itemp);
			
			pDate->iDirection=itemp;
			
			itemp=-1;
			rst.GetFieldValue("STATE",itemp);
			
			pDate->iState=itemp;
			
			szTemp.Empty();
			rst.GetFieldValue("DATATIME",szTemp);
			
			sprintf(pDate->Time,"%s",szTemp);
			
			fTemp=-1;
			rst.GetFieldValue("SPEED",fTemp);
			
			pDate->fSpeed=fTemp;
			int  iflag;
			rst.GetFieldValue("FLAG",iflag);
			
			pDate->iFlag=iflag;  
			aGpsDate.Add((CObject  *)pDate);
			rst.MoveNext();
		}		
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到gps数据失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
	
}
//end;

//得到设备的时间设置;
int CDBWork::GetEquimentSetTime(TEMP_EquimentTimeSet &asTimeSet)
{
	CString szSQL, szTemp;
	asTimeSet.RemoveAll();
	asTimeSet.SetSize(0);
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  EQIMENTTIMESET");		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到设备时间设置信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		while(!rst.IsEof())
		{
			CEquimentTimeSet  info;		
			rst.GetFieldValue("EQIMENTID",szTemp);
            info.szEquimentId=szTemp;

			rst.GetFieldValue("TEXT",szTemp);
	        info.szStr=szTemp;	
			asTimeSet.Add(info);
			rst.MoveNext();	
		}
	
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到设备时间设置信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

//更新时间设置信息
int CDBWork::UpdateEquimentTimeSet(CString szId, CString szText)
{
	szId.TrimLeft(),szId.TrimRight();
	if(szId.IsEmpty())
		return  DB_PAR_ERROR;
	CString   szSql,szTemp;
	CADORecordset rst(&m_Database);
    szSql.Format("select  *  from  EQIMENTTIMESET  where EQIMENTID='%s'",szId);
    


	try
	{
		//	szSql.Format("select  * from  GPSDATANEWEST ");
		if(!m_Database.Execute(szSql, rst))
		{
			return  DB_EXE_ERROR;
		}
		if(!rst.IsEof())
		{
			szSql.Format("update  EQIMENTTIMESET  set TEXT='%s'"
				"     where  EQIMENTID='%s' ",szText,szId);
		}
		else
		{
			szSql.Format("insert into EQIMENTTIMESET(EQIMENTID,TEXT)"
				"   values('%s','%s')",
				szId,szText);
		}
		if(!m_Database.Execute(szSql, rst))
		{
			szTemp.Format("写入EQIMENTTIMESET表失败信息失败.(SQL:%s)",szSql);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch (_com_error  &e)
	{
		szTemp.Format("写入EQIMENTTIMESET表失败信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSql);
		g_Log(szTemp);
		return DB_EXE_ERROR;
		
	}
	return DB_OK;

}

//更新用户的登陆信息;
int CDBWork::UpDataUserLog(CString szUserId, CString szTime, CString szIP, int iType)
{
	CString   szSql,szTemp;
	CADORecordset rst(&m_Database);	

	szSql.Format("insert into USERLOGIN_TEMP(USERID,IP,LOGTIME,ITYPE)"
		"  values('%s','%s','%s',%d)",szUserId,szIP,szTime,iType);
	try
	{
		//	szSql.Format("select  * from  GPSDATANEWEST ");
		if(!m_Database.Execute(szSql, rst))
		{
			szTemp.Format("写入USERLOGIN_TEMP表失败信息失败.(SQL:%s)",szSql);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch (_com_error  &e)
	{
		szTemp.Format("写入USERLOGIN_TEMP表失败信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSql);
		g_Log(szTemp);
		return DB_EXE_ERROR;
		
	}
	return DB_OK;

}
int CDBWork::DelSpeedAlarm(CString szKeyid)
{
	CString   szSql,szTemp;
	CADORecordset rst(&m_Database);
	szSql.Format("delete   from   SPEEDALARMSET  where KEYID='%s'",szKeyid);
	
	try
	{
		if(!m_Database.Execute(szSql))
		{
			
			return DB_EXE_ERROR;
		}
		
	}
	catch (_com_error  &e)
	{
		
		return DB_EXE_ERROR;
		
	}	
	return DB_OK;
	
}
int CDBWork::SetSpeedAlarm(CString szKeyid, int iSpeed1,int iSpeed2)
{
	
	CString   szSql,szTemp;
	CADORecordset rst(&m_Database);
	szSql.Format("insert into SPEEDALARMSET(KEYID,LSPEED,SPEED)"
		"  values('%s',%d,%d)",
		szKeyid,iSpeed1,iSpeed2);
	try
	{
		if(!m_Database.Execute(szSql))
		{
			
			return DB_EXE_ERROR;
		}
		
	}
	catch (_com_error  &e)
	{
		
		return DB_EXE_ERROR;
		
	}	
	return DB_OK;
}

int CDBWork::GetSpeedAlarm(TEMP_SpeedAlarmInfo &asSpeedAlarm)
{
	
	asSpeedAlarm.RemoveAll();
	asSpeedAlarm.SetSize(0);
	CString szSQL, szTemp;
	
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  SPEEDALARMSET");
		if(!m_Database.Execute(szSQL, rst))
		{
			
			return DB_EXE_ERROR;
		}
		
		while(!rst.IsEof())
		{
			CSpeedAlarmInfo  info;		
			
			rst.GetFieldValue("KEYID",info.szKeyId);		
			rst.GetFieldValue("LSPEED",info.iLSpeed);
			rst.GetFieldValue("SPEED",info.iHSpeed);
			asSpeedAlarm.Add(info);
			rst.MoveNext();
			
		}
		
		
	}
	catch(_com_error &e)
	{
		return DB_EXE_ERROR;
	}
	return  DB_OK;
	
}


int CDBWork::GetDiaoDu(TEMP_DioaduMessage &asDioaduMessage)
{

    asDioaduMessage.RemoveAll();
	asDioaduMessage.SetSize(0);
	CString szSQL, szTemp;
	
	
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  DIAODUTABLE");		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到设备报警接受手机信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		while(!rst.IsEof())
		{
		
			CDioaduMessage  info;
			rst.GetFieldValue("EQUIMENTID",info.szEquimentid);
			info.szEquimentid.TrimLeft(),info.szEquimentid.TrimRight();

			rst.GetFieldValue("EQUIMENTKEY",info.szKey);
			info.szKey.TrimLeft(),info.szKey.TrimRight();

			rst.GetFieldValue("SENDTEXT",info.szMessage);
			info.szMessage.TrimLeft(),info.szMessage.TrimRight();

			asDioaduMessage.Add(info);
			rst.MoveNext();
		
		}
	
		
		
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到设备报警接受手机信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

int CDBWork::DeleteDiaoDu(CString szEquimentId, CString szKey)
{

	CString   szSql,szTemp;
	szSql.Format("delete  from DIAODUTABLE  where  EQUIMENTID='%s'  and  EQUIMENTKEY='%s'",szEquimentId,szKey);
	try
	{
		if(!m_Database.Execute(szSql))
		{
	
			return DB_EXE_ERROR;
		}
		
	}
	catch (_com_error  &e)
	{
	
		return DB_EXE_ERROR;
		
	}
	return DB_OK;

}

int CDBWork::GetDianchanRunInfo(CStringArray  &asInfo)
{


	CString szSQL, szTemp;
    asInfo.RemoveAll();
	asInfo.SetSize(0);
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from  EQUIMENTINFO  where CARTYPE=0  and ISRUN=1");		
		if(!m_Database.Execute(szSQL, rst))
		{
	
			return DB_EXE_ERROR;
		}
		
		while(!rst.IsEof())
		{

			rst.GetFieldValue("INDUSTRYNUMBER",szTemp);
		
			asInfo.Add(szTemp);
            rst.MoveNext();
	        
	
		}
		rst.Close();
		
		
	}
	catch(_com_error &e)
	{

		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

int CDBWork::insertZXDianchanRun(CString szEquimentId, CString szCommandId, CString szTimeLog, CString szText,int  iType)
{
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		
		
		int  istate=0;
		szSQL.Format("insert into  DIANCHANRUNNINFO(EQUIMENTID,TIMELOG,TEXT,COMMANDID,ITYPE)"
			"  values('%s','%s','%s','%s',%d)",szEquimentId,szTimeLog,szText,szCommandId,iType);
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("插入电铲调度信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("插入电铲调度信息失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

int CDBWork::UpdataMessageReTime(CString szEquimentId, CString szTimeLog, CString szRTime,CString  szCommdid)
{

	CString szRTime2;
	GetmaxDianChanRime(szEquimentId, szRTime2);
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		
		
		int  istate=0;
		szSQL.Format("update  DIANCHANRUNNINFO  set RETIME='%s'  where  EQUIMENTID='%s' and  TIMELOG='%s'  ",szRTime,szEquimentId,szRTime2);
		g_Log(szSQL);
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("更新回应调度信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("更新回应调度信息失败.(Description:%s, SQL:%s)",szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;

}

int CDBWork::GetmaxDianChanRime(CString szEquimentid, CString &szTimelog)
{


	CString szSQL, szTemp;
	
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select max(TIMELOG) as timelog from  DIANCHANRUNNINFO  where EQUIMENTID='%s'",szEquimentid);		
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到设备报警接受手机信息失败.(SQL:%s)",szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
		
		if(!rst.IsEof())
		{
		
			rst.GetFieldValue("timelog",szTimelog);
		
			rst.Close();
		}
		else
			return DB_EXE_ERROR;
		
		
		
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到设备报警接受手机信息失败.(Description:%s, SQL:%s)",
			(char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}
	return  DB_OK;
}

int CDBWork::GetAreaInfo(map<CString, CAreaInfo> &mapAreaInfo)
{
	CString szSQL, szTemp;
	int  nAreType, nValid;
	double dSpeed = 0;

	mapAreaInfo.clear();

	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("select * from AreaInfo");
		if(!m_Database.Execute(szSQL, rst))
		{
			szTemp.Format("得到区域信息失败.(SQL:%s)", szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}

		while(!rst.IsEof())
		{
			CAreaInfo area;

			rst.GetFieldValue("ID", szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(area.ID, "%s", szTemp);
				area.ID[szTemp.GetLength()]=0;
			}

			rst.GetFieldValue("NAME", szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(area.NAME, "%s", szTemp);
				area.NAME[szTemp.GetLength()]=0;
			}

			rst.GetFieldValue("TYPE", nAreType);
			area.TYPE =nAreType;

			rst.GetFieldValue("VALID", nValid);
			area.VALID = nValid;

			rst.GetFieldValue("SPEED", dSpeed);
			area.SPEED = dSpeed;

			rst.GetFieldValue("PATH", szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(area.PATH, "%s", szTemp);
				area.PATH[szTemp.GetLength()]=0;
			}

			rst.GetFieldValue("CREATETIME",szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(area.CREATETIME, "%s", szTemp);
				area.CREATETIME[szTemp.GetLength()]=0;
			}

			rst.GetFieldValue("INVALIDTIME", szTemp);
			if(!szTemp.IsEmpty())
			{
				szTemp.TrimLeft();
				szTemp.TrimRight();
				sprintf(area.INVALIDTIME, "%s", szTemp);
				area.INVALIDTIME[szTemp.GetLength()]=0;
			}

			mapAreaInfo.insert(pair<CString, CAreaInfo>(area.ID, area));

			rst.MoveNext();	
		}
	}
	catch(_com_error &e)
	{
		szTemp.Format("得到区域信息失败.(Description:%s, SQL:%s)", (char *)e.Description(), szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}

	return  DB_OK;
}

int CDBWork::AddEventInfo(CEventInfo info)
{
	CString szSQL, szTemp;
	CADORecordset rst(&m_Database);
	try
	{
		szSQL.Format("insert into  EventInfo(EquipmentID, PeerID, Type, BeginTime, EndTime, AreaName, Speed, SpeedLimit) values('%s', '%s', %d, '%s', '%s', '%s', %.2f, %.2f)",
			info.EQUIPMENTID, info.PEERID, info.TYPE, info.BEGINTIME, info.ENDTIME, info.AREANAME, info.SPEED, info.SpeedLimit);
		if(!m_Database.Execute(szSQL))
		{
			szTemp.Format("插入事件信息失败.(SQL:%s)", szSQL);
			g_Log(szTemp);
			return DB_EXE_ERROR;
		}
	}
	catch(_com_error &e)
	{
		szTemp.Format("插入事件信息失败.(Description:%s, SQL:%s)", szSQL);
		g_Log(szTemp);
		return DB_EXE_ERROR;
	}

	return  DB_OK;
}
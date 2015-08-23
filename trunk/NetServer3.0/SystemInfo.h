// SystemInfo.h: interface for the CSystemInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYSTEMINFO_H__BDC0ABB6_140A_4A36_A765_05CD9865C002__INCLUDED_)
#define AFX_SYSTEMINFO_H__BDC0ABB6_140A_4A36_A765_05CD9865C002__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//设备端口的信息;   netprot
#define   MTJJA_PORT  6668   //明天电子的家家设备
#define   MTGPS_PORT  5254   //明天电子的GPS设备
#define   HQGPS_PORT  6666   //华强的gps设备
#define   TQGPS_PORT  8800 
#define   GTQGPS_PORT  4254  //TQ无行业应用
//设备类型的定义;
#define   EQUIPMENT_HQGPS   1
#define   EQUIPMENT_TQGPS   2
#define   EQUIPMENT_MTJJA   3
#define   EQUIPMENT_MTGPS   4
#define   EQUIPMENT_GTQGPS   5  //TQ无行业应用
//设备启动信息
#define   MAXSERVERTYPE     6



class CSystemInfo  
{
public:
	static BOOL GetIsServerStart(int  iType);
	CString GetDBUser();
	static   CString GetDBName() ;
	static   CString GetDBPWSD() ;
	static   int   GetPort()  ;
	CSystemInfo();
	virtual ~CSystemInfo();
private:
	static  short  m_iPort;  //系统启动监听客户端的口;
	static  char   m_szIP[50];   //计算机的ip地址;
	static  char   m_szDBName[50];  //要连接的db名称
	static  char   m_szDbPassWsd[50];//db的密码;
	static  char   m_szUser[50];// 数据库的登陆名
   //用来设置启动的服务的;
	static  BOOL    m_BStart[MAXSERVERTYPE];;  


};

#endif // !defined(AFX_SYSTEMINFO_H__BDC0ABB6_140A_4A36_A765_05CD9865C002__INCLUDED_)

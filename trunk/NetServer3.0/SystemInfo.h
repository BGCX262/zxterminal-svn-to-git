// SystemInfo.h: interface for the CSystemInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYSTEMINFO_H__BDC0ABB6_140A_4A36_A765_05CD9865C002__INCLUDED_)
#define AFX_SYSTEMINFO_H__BDC0ABB6_140A_4A36_A765_05CD9865C002__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//�豸�˿ڵ���Ϣ;   netprot
#define   MTJJA_PORT  6668   //������ӵļҼ��豸
#define   MTGPS_PORT  5254   //������ӵ�GPS�豸
#define   HQGPS_PORT  6666   //��ǿ��gps�豸
#define   TQGPS_PORT  8800 
#define   GTQGPS_PORT  4254  //TQ����ҵӦ��
//�豸���͵Ķ���;
#define   EQUIPMENT_HQGPS   1
#define   EQUIPMENT_TQGPS   2
#define   EQUIPMENT_MTJJA   3
#define   EQUIPMENT_MTGPS   4
#define   EQUIPMENT_GTQGPS   5  //TQ����ҵӦ��
//�豸������Ϣ
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
	static  short  m_iPort;  //ϵͳ���������ͻ��˵Ŀ�;
	static  char   m_szIP[50];   //�������ip��ַ;
	static  char   m_szDBName[50];  //Ҫ���ӵ�db����
	static  char   m_szDbPassWsd[50];//db������;
	static  char   m_szUser[50];// ���ݿ�ĵ�½��
   //�������������ķ����;
	static  BOOL    m_BStart[MAXSERVERTYPE];;  


};

#endif // !defined(AFX_SYSTEMINFO_H__BDC0ABB6_140A_4A36_A765_05CD9865C002__INCLUDED_)

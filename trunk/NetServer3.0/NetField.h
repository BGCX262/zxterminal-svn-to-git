
/*����������������ص�Э��ṹ*/
/*version  1.0*/
/*20060802  edited  by  ton*/

#ifndef  _NETFIELD_H_
#define  _NETFIELD_H_
#include "NetNode.h"
#include <afxtempl.h>
#include "NetNode.h"
#include "Winsock2.h"
#include <afxmt.h>

#define   MAXEQUIEMNTIDARRAYLEN   160
#define   EQUIMENTIDLEN  11
//������Ϣ���ͣ��豸�����ͣ�
#define    MSGTYPE_COMMON  0   //��ͨ����Ϣ���豸�޹ص���Ϣ;
#define    MSGTYPE_GPS  1   //gps  ���͵��豸;
#define    MSGTYPE_GUARD   2  //gps  ���͵��豸;



const  int  NETDATAMAXLEN=1024;  //������Ϣ��ĳ���;
const  UINT VERSION=0x0001;  //��ǰ������İ汾��;

#define  MSGSIGN    0x24
//�������ݰ��Ľṹ;


//����Ϣ������;
typedef  enum {
	MAINMSG_NULL=0x00000000,
	MAINMSG_USERLOGIN=0x00000001,  //�û���½;
	MAINMSG_USERALLEQUIID=0x00000002,  //�����û����е��豸��ID;
	MAINMSG_EQUIMENTINFO=0x00000003,  //�����豸����Ϣ;
	MAINMSG_GPSDATATOCLIENT=0x00000004,//GPS����
	MAINMSG_GUARDDATATOCLIENT=0x00000005,//guard  data

	//guard  comm;
	MAINMSG_GUARDCOMMAND=0x00000006, //�����豸������;
	MAINMSG_HEARTBEAT=0x00000007, //��������;
	MAINMSG_GPSCOMMAND=0x00000008, //GPS�豸������;
	//addede 20061218
	MAINMSG_DATADOWNLOAD=0x00000009, //GPS���ݵ�����;
	//20070226
	MAINMSG_CHANGEUSERPWS=0x0000000A, //�û��������;
	MAINMSG_CHANGEUSERVALIDATE=0x0000000B, //�û���֤�Ÿ���;
	MAINMSG_CLIENTQUERY=0x0000000C, //�ͻ���ѯ
	//20070228
	MAINMSG_USERPOWER=0x0000000D, //Ȩ����Ϣ
	MAINMSG_SOFTVERSION=0x0000000E, //�汾��Ϣ

	MAINMSG_OPATERRESULT=0x00000010, //������
	
	MAINMSG_INFORMCLIENT=0x00000011, //֪ͨ�ͻ� ��������
	//20070301
	MAINMSG_CTRTELDATEDOWN=0x00000012, //�����ֻ���Ϣ����
	MAINMSG_SENDTELDATEDOWN=0x00000013, //���������ֻ���Ϣ����
	MAINMSG_ALARMINFODATEDOWN=0x00000014, //������Ϣ��������;
	MAINMSG_FEEINFODATEDOWN=0x00000015, //������Ϣ����
	MAINMSG_UPDATECTRTEL=0x00000016,  //���¿����ֻ���Ϣ
	MAINMSG_UPDATESENDTEL=0x00000017,  //���½��ձ������ֻ���Ϣ;
	MAINMSG_UPDATEALARMINFO=0x00000018,  //���±�����Ϣ
	MAINMSG_DOWNLOADMAP=0x00000019,  //���ص�ͼ�ɸ����б�;
	MAINMSG_GPSMESSAGETOCLIENT=0x0000001A,//GPS����Ϣ
	MAINMSG_GPSDATAQUERY=0x0000001B,//����gps����;
    MAINMSG_QUERYUSERNEWESTGPSDATA=0x0000001C,//�����û�������gps����;
	MAINMSG_GOOGLEMAPQUERY=0x0000001D,//google ��ͼ����;


	

}MIANMSGTYPE;
//����Ϣ����;
typedef  enum {
	SUBMSG_NULL=0x00000000,
	SUBMSG_REQUES=0x00000010,  //��������;
	SUBMSG_REQUES_ERROR=0x00000020,  //����ʧ��;
	SUBMSG_REQUES_SUCESSFUL=0x00000021,  //����ɹ�;
	SUBMSG_REQUES_USERHAVELONGIN=0x00000022,  //�û��Ѿ���½;
	SUBMSG_COMMAND_REQUES=0x00000030,  //�����
	SUBMSG_COMMAND_SUCESSFUL=0x00000031,//����ͳɹ�;
	SUBMSG_COMMAND_ERROR=0x00000032,//�����ʧ��;
	//added 20061218
	SUBMSG_DATADOWNLOAD_ERROR=0x01000032,  //��������ʧ��
	SUBMSG_DATADOWNLOAD_NORECORD=0x01000033,  //���������޶�Ӧ������;
	SUBMSG_DATADOWNLOAD_FINISHED=0x01000034,  //���������޽���;
	SUBMSG_DATADOWNLOAD_BODY=0x01000035,  //��������;




}SUBMSGTYPE;
//1:��Ϣͷ
typedef  struct tagNetMsgHead
{
	BYTE  BeginHead;  //Э�鿪ʼ��־;  0x24(*);
	UINT  Version;    //�汾��;
	BYTE  MsgType;    //��Ϣ�ķ���  0��һ�������  �����������豸�Ŀ�������������Ҫ���Ƶ��豸�����ͣ�
	int   NetMsgLen;  //��Ϣ �ĳ��ȣ�
	MIANMSGTYPE   MainMsgType; //����Ϣ����;
	SUBMSGTYPE  SubMsgType;  //����Ϣ���� 1x:����  2x���ظ�;
	int   ContentLen;  //��Ϣ�峤��;
	BYTE  EndHead;  //Э��������־;  0x24(*);
	tagNetMsgHead()
	{
		BeginHead=(BYTE)0x24;
		MsgType=(BYTE)0x0;  //Ĭ��һ��ĺ��豸�޹ص���Ϣ;
		Version=VERSION;
		NetMsgLen=32;  //Ĭ��ʱ��û����Ϣ��;
		ContentLen=0;
		EndHead=(BYTE)0x24;
	}

}NetMsgHead,*pNetMsgHead;

//���ݵ��߳���Ϣ�Ľṹ
typedef   struct  tagThreadMsg{
	int  fd;  //��Ӧ��fd;
	char *pBuf;
	int  iLen;   //��Ϣ�ĳ���
//	char  ip[20];
	tagThreadMsg()
	{
		fd=0;
		pBuf=NULL;
		iLen=0;
	}

}ThreadMsg,*pThreadMsg;
typedef   CArray<ThreadMsg,ThreadMsg&>TEMP_THREADMSG;

//��������Ĳ�ͬ���������ݵĽṹ;
//�û���½��Ϣ;
typedef  struct  tagUserLogin
{
	char  szUserId[21];  //�û���id ���20���ֽ�
	char  szPWS[21];    //�û�������;
	tagUserLogin()
	{
		ZeroMemory(szUserId,21);
		ZeroMemory(szPWS,21);
	}

}UserLogin;
//end;
//�û��ĵ�½ע����Ϣ;
typedef  struct tagClintNode
{
	int  fd;  //sokcet  index;
	struct  sockaddr_in  addr;  //addresss;
	char   UserID[30];   //userid;
	CString  szLoginTime;
	int      iRecvHeartbeatCount;   //��������  addede  20070325
	tagClintNode()
	{

		iRecvHeartbeatCount=0;  //��������  addede  20070325
		fd=INVALID_SOCKET;
		ZeroMemory(&addr,sizeof(addr));
		addr.sin_family=AF_INET;
		ZeroMemory(UserID,30);
		COleDateTime   now=COleDateTime::GetCurrentTime();
		szLoginTime.Format("%s",now.Format("%Y-%m-%d %H:%M:%S"));
	}
	
}ClintNode;

//���� ��������
class   CDownLoadCommand
{
public:
	int   DownLoadNumber;
	char  userId[30];  //added   by  tgc 20070420;  
	char  asEquimentId[MAXEQUIEMNTIDARRAYLEN];   //������;�ֿ�;
	int   iEquimentType;// 1 gps  2:guard,  0:��Ч;
	int   iCount;  //�豸������;
	char   BeginTime[20];
	char    EndTime[20];
	int    iType;//���ص�ʱ����������ݷ���;  0:ȫ����2�ٶȲ�Ϊ0�ĵ㣬2��״̬��Ӧ�ĵ�;
	CDownLoadCommand()
	{
		DownLoadNumber=0;  //added 20070420
		memset(userId,0,30);
		memset(asEquimentId,0,MAXEQUIEMNTIDARRAYLEN);
		iCount=0;
		memset(BeginTime,0,20);
		memset(EndTime,0,20);
		iType=0;
		iEquimentType=0;
	}
	
};
//������������Ļ�Ӧ;
class  CDownLoadGpsDateResult
{
public:
	int   DownLoadNumber;   //����ı��
};



//�������صĸ�ʽ
typedef    struct tagDataDownLaod
{
	BYTE  BeginData;  //'*'
	int  iTotalCount;//�������ݵĸ���;
	int  iOrder;//��ǰ���ݵĸ���;
	int   iLen;//���ݵĳ���;
	int   iCount;//GPS���ݵĸ���;
	BYTE  Isvalid;
	char  GpsData[250];
	BYTE  EndData;//'*'	
	tagDataDownLaod()
	{
		BeginData='*';
		EndData='*';
		memset(GpsData,0,250);
	}
}DATADOWNLAOD;
//google ��ͼ������
typedef    struct tagQueryGoogleMap
{
	char   UserId[50];
	char    EquimentID[15];
	tagQueryGoogleMap()
	{
		memset(UserId,0,50);
		memset(EquimentID,0,15);

	}


}QueryGoogleMap;

 

#endif






















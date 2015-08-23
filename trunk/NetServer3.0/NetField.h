
/*定义了所有网络相关的协议结构*/
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
//网络消息类型（设备的类型）
#define    MSGTYPE_COMMON  0   //普通的信息和设备无关的信息;
#define    MSGTYPE_GPS  1   //gps  类型的设备;
#define    MSGTYPE_GUARD   2  //gps  类型的设备;



const  int  NETDATAMAXLEN=1024;  //最大的消息体的长度;
const  UINT VERSION=0x0001;  //当前的软件的版本号;

#define  MSGSIGN    0x24
//网络数据包的结构;


//主消息的类型;
typedef  enum {
	MAINMSG_NULL=0x00000000,
	MAINMSG_USERLOGIN=0x00000001,  //用户登陆;
	MAINMSG_USERALLEQUIID=0x00000002,  //请求用户所有的设备的ID;
	MAINMSG_EQUIMENTINFO=0x00000003,  //请求设备的信息;
	MAINMSG_GPSDATATOCLIENT=0x00000004,//GPS数据
	MAINMSG_GUARDDATATOCLIENT=0x00000005,//guard  data

	//guard  comm;
	MAINMSG_GUARDCOMMAND=0x00000006, //安方设备的命令;
	MAINMSG_HEARTBEAT=0x00000007, //心跳测试;
	MAINMSG_GPSCOMMAND=0x00000008, //GPS设备的命令;
	//addede 20061218
	MAINMSG_DATADOWNLOAD=0x00000009, //GPS数据的下载;
	//20070226
	MAINMSG_CHANGEUSERPWS=0x0000000A, //用户密码更改;
	MAINMSG_CHANGEUSERVALIDATE=0x0000000B, //用户验证号更改;
	MAINMSG_CLIENTQUERY=0x0000000C, //客户查询
	//20070228
	MAINMSG_USERPOWER=0x0000000D, //权限信息
	MAINMSG_SOFTVERSION=0x0000000E, //版本信息

	MAINMSG_OPATERRESULT=0x00000010, //处理结果
	
	MAINMSG_INFORMCLIENT=0x00000011, //通知客户 和任务安排
	//20070301
	MAINMSG_CTRTELDATEDOWN=0x00000012, //控制手机信息下载
	MAINMSG_SENDTELDATEDOWN=0x00000013, //报警接受手机信息下载
	MAINMSG_ALARMINFODATEDOWN=0x00000014, //报警信息描述下载;
	MAINMSG_FEEINFODATEDOWN=0x00000015, //费用信息下载
	MAINMSG_UPDATECTRTEL=0x00000016,  //更新控制手机信息
	MAINMSG_UPDATESENDTEL=0x00000017,  //更新接收报警的手机信息;
	MAINMSG_UPDATEALARMINFO=0x00000018,  //更新报警信息
	MAINMSG_DOWNLOADMAP=0x00000019,  //下载地图可更新列表;
	MAINMSG_GPSMESSAGETOCLIENT=0x0000001A,//GPS短信息
	MAINMSG_GPSDATAQUERY=0x0000001B,//请求gps数据;
    MAINMSG_QUERYUSERNEWESTGPSDATA=0x0000001C,//请求用户的最新gps数据;
	MAINMSG_GOOGLEMAPQUERY=0x0000001D,//google 地图请求;


	

}MIANMSGTYPE;
//子消息类型;
typedef  enum {
	SUBMSG_NULL=0x00000000,
	SUBMSG_REQUES=0x00000010,  //操作请求;
	SUBMSG_REQUES_ERROR=0x00000020,  //请求失败;
	SUBMSG_REQUES_SUCESSFUL=0x00000021,  //请求成功;
	SUBMSG_REQUES_USERHAVELONGIN=0x00000022,  //用户已经登陆;
	SUBMSG_COMMAND_REQUES=0x00000030,  //命令发送
	SUBMSG_COMMAND_SUCESSFUL=0x00000031,//命令发送成功;
	SUBMSG_COMMAND_ERROR=0x00000032,//命令发送失败;
	//added 20061218
	SUBMSG_DATADOWNLOAD_ERROR=0x01000032,  //数据下载失败
	SUBMSG_DATADOWNLOAD_NORECORD=0x01000033,  //数据下载无对应的数据;
	SUBMSG_DATADOWNLOAD_FINISHED=0x01000034,  //数据下载无结束;
	SUBMSG_DATADOWNLOAD_BODY=0x01000035,  //数据下载;




}SUBMSGTYPE;
//1:消息头
typedef  struct tagNetMsgHead
{
	BYTE  BeginHead;  //协议开始标志;  0x24(*);
	UINT  Version;    //版本号;
	BYTE  MsgType;    //消息的分类  0：一般的请求  其他：若是设备的控制请求，他代表要控制的设备的类型；
	int   NetMsgLen;  //消息 的长度；
	MIANMSGTYPE   MainMsgType; //主消息类型;
	SUBMSGTYPE  SubMsgType;  //子消息类型 1x:请求  2x：回复;
	int   ContentLen;  //消息体长度;
	BYTE  EndHead;  //协议界结束标志;  0x24(*);
	tagNetMsgHead()
	{
		BeginHead=(BYTE)0x24;
		MsgType=(BYTE)0x0;  //默认一般的和设备无关的消息;
		Version=VERSION;
		NetMsgLen=32;  //默认时候没有消息体;
		ContentLen=0;
		EndHead=(BYTE)0x24;
	}

}NetMsgHead,*pNetMsgHead;

//传递的线程消息的结构
typedef   struct  tagThreadMsg{
	int  fd;  //对应的fd;
	char *pBuf;
	int  iLen;   //消息的长度
//	char  ip[20];
	tagThreadMsg()
	{
		fd=0;
		pBuf=NULL;
		iLen=0;
	}

}ThreadMsg,*pThreadMsg;
typedef   CArray<ThreadMsg,ThreadMsg&>TEMP_THREADMSG;

//根据请求的不同所带的数据的结构;
//用户登陆信息;
typedef  struct  tagUserLogin
{
	char  szUserId[21];  //用户的id 最长是20个字节
	char  szPWS[21];    //用户的密码;
	tagUserLogin()
	{
		ZeroMemory(szUserId,21);
		ZeroMemory(szPWS,21);
	}

}UserLogin;
//end;
//用户的登陆注册信息;
typedef  struct tagClintNode
{
	int  fd;  //sokcet  index;
	struct  sockaddr_in  addr;  //addresss;
	char   UserID[30];   //userid;
	CString  szLoginTime;
	int      iRecvHeartbeatCount;   //心跳记数  addede  20070325
	tagClintNode()
	{

		iRecvHeartbeatCount=0;  //心跳记数  addede  20070325
		fd=INVALID_SOCKET;
		ZeroMemory(&addr,sizeof(addr));
		addr.sin_family=AF_INET;
		ZeroMemory(UserID,30);
		COleDateTime   now=COleDateTime::GetCurrentTime();
		szLoginTime.Format("%s",now.Format("%Y-%m-%d %H:%M:%S"));
	}
	
}ClintNode;

//数据 下载命令
class   CDownLoadCommand
{
public:
	int   DownLoadNumber;
	char  userId[30];  //added   by  tgc 20070420;  
	char  asEquimentId[MAXEQUIEMNTIDARRAYLEN];   //数据用;分开;
	int   iEquimentType;// 1 gps  2:guard,  0:无效;
	int   iCount;  //设备的数量;
	char   BeginTime[20];
	char    EndTime[20];
	int    iType;//下载的时候请求的数据非类;  0:全部：2速度不为0的点，2有状态回应的点;
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
//数据下载任务的回应;
class  CDownLoadGpsDateResult
{
public:
	int   DownLoadNumber;   //任务的编号
};



//数据下载的格式
typedef    struct tagDataDownLaod
{
	BYTE  BeginData;  //'*'
	int  iTotalCount;//所有数据的个数;
	int  iOrder;//当前数据的个数;
	int   iLen;//数据的长度;
	int   iCount;//GPS数据的个数;
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
//google 地图的请求
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






















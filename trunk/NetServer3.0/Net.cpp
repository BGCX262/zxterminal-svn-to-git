
#include "stdafx.h"
#include "Net.h"
#include "WinSock.h"


int CreateTcpSocketServer(const  char  *szip,const short iPort)
{
	SOCKET   fd;
    struct  sockaddr_in     addr;
	memset(&addr,0,sizeof(addr));	
	addr.sin_family=AF_INET;
	addr.sin_port=htons(iPort);
	addr.sin_addr.s_addr=INADDR_ANY;
	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd==INVALID_SOCKET)
		return   -1;
    int iRet=-1;
	iRet=bind(fd,(SOCKADDR  *) &addr,sizeof(addr));
	if(iRet==SOCKET_ERROR)
	{
		closesocket(fd);
		return   -1;
	}
	iRet=listen(fd,SOMAXCONN);
	if(iRet==SOCKET_ERROR)
	{
		closesocket(fd);
		return -1;
	}
	return  fd;
}

int CreateTcpSocketClient(const  char  *szip,const short iPort)
{
	SOCKET   fd;
    struct  sockaddr_in     addr;
	memset(&addr,0,sizeof(addr));	
	addr.sin_family=AF_INET;
	addr.sin_port=htons(iPort);
	addr.sin_addr.s_addr=inet_addr(szip);
	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd==INVALID_SOCKET)
		return  -1;
	int iRet=connect(fd,(SOCKADDR *)&addr,sizeof(addr));
	if(iRet==SOCKET_ERROR)
		return  -1;
	return  fd;
}



int CreateUdpScoket(const  char  *szip,const short iPort)
{
	
	SOCKET   fd;
    struct  sockaddr_in     addr;
	memset(&addr,0,sizeof(addr));	
	addr.sin_family=AF_INET;
	addr.sin_port=htons(iPort);
	addr.sin_addr.s_addr=INADDR_ANY;
	fd=socket(AF_INET,SOCK_DGRAM,0);
	if(fd==INVALID_SOCKET)
	{
		int iError=WSAGetLastError();
		return  -1;
	}
	int  iRet=bind(fd,(SOCKADDR *)&addr,sizeof(addr));
	if(iRet==SOCKET_ERROR)
	{
		int iError=WSAGetLastError();
		closesocket(fd);
		return  -1;
	}
	return  fd;
}

//发送网络数据包  -1，失败;  返回发送出去的字节;
int SendToNetMsg(int  fd,char *pBuf,int iLen)
{
	if(fd<=0  ||!pBuf  ||  iLen<=0)
	{
		return  -1;  //发送数据失败
	}
	fd_set  write_fd;
	FD_ZERO(&write_fd);
	FD_SET(fd,&write_fd);
	timeval  tv;
	tv.tv_sec=1;
	tv.tv_usec=0;
	int  iLeft=iLen; //剩余的为发送的数据;

	while(iLeft>0)
	{
		int  iRet=select(fd+1,NULL,&write_fd,NULL,&tv);
		if(iRet==SOCKET_ERROR)
		{//发生了错误
			return  -1;
		}
		if(iRet==0)
			continue;
		if(iRet>0)
		{
			int iSend=send(fd,&pBuf[iLeft-iLeft],iLeft,0);
			if(iSend<0  || iSend==SOCKET_ERROR)
				return  -1;
			iLeft-=iSend;
		}
	}
	return  iLen-iLeft;
}

void ReleaseSocket(int  fd)
{
	int  ierror=closesocket(fd);
	if(ierror==SOCKET_ERROR)
	{
		int i=WSAGetLastError();
		TRACE("关闭socket错误:%d\n", i);
	}
	return ;
}























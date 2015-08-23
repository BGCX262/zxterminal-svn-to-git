

/* 网络的公共函数*/
/* version 1.0*/
/* create  by   duck.ton  20060708*/
#ifndef  _NET_H_
#define  _NET_H_




int CreateTcpSocketServer(const  char  *szip,const short iPort);
int CreateTcpSocketClient(const  char  *szip,const short iPort);

int CreateUdpScoket(const  char  *szip,const short iPort);
int SendToNetMsg(int  fd,char *pBuf,int iLen);
void ReleaseSocket(int  fd);

#endif






















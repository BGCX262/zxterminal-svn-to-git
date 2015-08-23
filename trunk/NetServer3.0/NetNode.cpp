// NetNode.cpp: implementation of the CNetNode class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetNode.h"
#include "MemoryPool.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


extern CMemoryPool   g_MemPool;  //�õ�һ���̳߳�;

CNetNode::CNetNode()
{

	iTotalLen=BUFLEN;
	iBufLen=0;
	iTotalMsgLen=MSGMINLEN;
//	pBuf=g_MemPool.GetBuf();
	pBuf=NULL;

}

CNetNode::CNetNode(int  fd)
{
	iTotalLen=BUFLEN;
    iBufLen=0;
	this->fd=fd;
	iTotalMsgLen=MSGMINLEN;
//	pBuf=g_MemPool.GetBuf();
	pBuf=NULL;
	
}

CNetNode::~CNetNode()
{

}
void CNetNode::TuneNetNod(CNetNode *pNetNode)
{//


}

void CNetNode::SetNetNode(int ibufLen)
{
	this->iBufLen+=iBufLen;
}

//�õ����еĿռ�ĳ��� 
int CNetNode::GetFreeBufLen() const
{
	return  this->iTotalLen-iBufLen;
}

void CNetNode::SetBufLen(int  iNewLen)
{
	this->iBufLen+=iNewLen;
}

void CNetNode::ClaerNodeData()
{

}

void CNetNode::Inti()
{
	iTotalLen = BUFLEN;
	iBufLen = 0;
	iTotalMsgLen = MSGMINLEN;
	//pBuf=g_MemPool.GetBuf();
	pBuf = NULL;
}

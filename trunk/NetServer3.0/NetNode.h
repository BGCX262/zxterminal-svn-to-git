// NetNode.h: interface for the CNetNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETNODE_H__4C0833E4_C9DE_48F4_B79F_4C172A49D170__INCLUDED_)
#define AFX_NETNODE_H__4C0833E4_C9DE_48F4_B79F_4C172A49D170__INCLUDED_

/*主要管理系统中的tcp  socket 的连接  和数据的管理*/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define   BUFLEN  512
#define   MSGMINLEN   32  //  包的最小长度;


class CNetNode  
{
public:
	void Inti();
	void ClaerNodeData();
	void SetBufLen(int  iNewLen  );
	//判断是不是完整的消息包，如果是给出消息结束的位置;
	int GetFreeBufLen()   const;
	void SetNetNode(int  ibufLen);   //调整参数;
	void TuneNetNod(CNetNode *pNetNode);  //接受过数据以后，对他的内存等进行调整;
	CNetNode();
	CNetNode(int  fd);
	virtual ~CNetNode();
	int  fd;
	char  *pBuf;
	int   iTotalLen;   //记录buf的总的长度;
	int    iBufLen;     //显示当前数据的长度;
	int    iTotalMsgLen;
//	int   m_CurrentPointAt; //用来记录当前可以用的第一个位置，写的时候要检测他是不是合法的数据;
	
};

#endif // !defined(AFX_NETNODE_H__4C0833E4_C9DE_48F4_B79F_4C172A49D170__INCLUDED_)






















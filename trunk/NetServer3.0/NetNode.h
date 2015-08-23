// NetNode.h: interface for the CNetNode class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NETNODE_H__4C0833E4_C9DE_48F4_B79F_4C172A49D170__INCLUDED_)
#define AFX_NETNODE_H__4C0833E4_C9DE_48F4_B79F_4C172A49D170__INCLUDED_

/*��Ҫ����ϵͳ�е�tcp  socket ������  �����ݵĹ���*/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define   BUFLEN  512
#define   MSGMINLEN   32  //  ������С����;


class CNetNode  
{
public:
	void Inti();
	void ClaerNodeData();
	void SetBufLen(int  iNewLen  );
	//�ж��ǲ�����������Ϣ��������Ǹ�����Ϣ������λ��;
	int GetFreeBufLen()   const;
	void SetNetNode(int  ibufLen);   //��������;
	void TuneNetNod(CNetNode *pNetNode);  //���ܹ������Ժ󣬶������ڴ�Ƚ��е���;
	CNetNode();
	CNetNode(int  fd);
	virtual ~CNetNode();
	int  fd;
	char  *pBuf;
	int   iTotalLen;   //��¼buf���ܵĳ���;
	int    iBufLen;     //��ʾ��ǰ���ݵĳ���;
	int    iTotalMsgLen;
//	int   m_CurrentPointAt; //������¼��ǰ�����õĵ�һ��λ�ã�д��ʱ��Ҫ������ǲ��ǺϷ�������;
	
};

#endif // !defined(AFX_NETNODE_H__4C0833E4_C9DE_48F4_B79F_4C172A49D170__INCLUDED_)






















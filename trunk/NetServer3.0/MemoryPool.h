// MemoryPool.h: interface for the CMemoryPool class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMORYPOOL_H__59FA3BA8_8E33_4B42_AE90_F29287C97567__INCLUDED_)
#define AFX_MEMORYPOOL_H__59FA3BA8_8E33_4B42_AE90_F29287C97567__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*�ڴ��*/
#define   MAXPOOLSIZE  1024
#define   POOLBUFLEN   512
class CMemoryPool  
{
public:
	char  * GetBuf();
	void ReleaseBuf(char *pbuf);
	BOOL InitPool(int iSize=MAXPOOLSIZE);  //��ʼ�����ڴ�Ŀռ�;
	void ReleasePool();
	CMemoryPool();
	virtual ~CMemoryPool();
	int m_iPoolSize;   //�ڴ�صĴ�С;
	char  *pPOOL[MAXPOOLSIZE];
//#ifdef   _DEBUG
	int   m_NewCount;
	int   m_poolusecount;
//#endif

};

#endif // !defined(AFX_MEMORYPOOL_H__59FA3BA8_8E33_4B42_AE90_F29287C97567__INCLUDED_)

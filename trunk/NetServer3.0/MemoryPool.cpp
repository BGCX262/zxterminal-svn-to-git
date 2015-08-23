// MemoryPool.cpp: implementation of the CMemoryPool class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MemoryPool.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemoryPool::CMemoryPool()
{
  m_NewCount = 0;
  m_poolusecount = 0;
  InitPool();
}

CMemoryPool::~CMemoryPool()
{
}

BOOL CMemoryPool::InitPool(int iSize)
{
	if(iSize <= 0)
	{
		return  FALSE;
	}

	for(int i = 0; i < iSize; i ++)
	{
		pPOOL[i] = new  char[POOLBUFLEN];
		if(pPOOL[i] == NULL)
		{
			return  FALSE;
		}

		m_iPoolSize ++;
	    ZeroMemory(pPOOL[i], POOLBUFLEN);
	}

	return  TRUE;
}

//�黹�ڴ浽�ڴ��;
void CMemoryPool::ReleaseBuf(char* pbuf)
{
	if(!pbuf)
	{
		return;
	}

	BOOL Bfind = FALSE;
	for(int i = 0;i < m_iPoolSize; i ++)
	{
		if(pPOOL[i] == NULL)
		{
			Bfind = TRUE;
			pPOOL[i] = pbuf;
			m_poolusecount --;
			break;
		}
	}
	if(!Bfind)
	{
		//����Ҳ�������ɾ���ڴ�ռ�;
		delete []pbuf;
	}
}

//�õ����е��ڴ�;  û�п��е��ڴ��������
char* CMemoryPool::GetBuf()
{
    BOOL bFind = FALSE;
	for(int i = 0;i < m_iPoolSize; i ++)
	{
		if(pPOOL[i] != NULL)
		{
			//�ҵ����е��ڴ�;
			bFind = TRUE;
			char* p = pPOOL[i];
			memset(p, 0, POOLBUFLEN);
			pPOOL[i] = NULL;
			m_poolusecount ++;
			return p;
		}
	}

	if(!bFind)
	{
		m_NewCount ++;
		char* p = new char[POOLBUFLEN];
		if(p != NULL)
		{
			return p;
		}
	}

	return  NULL;
}

void CMemoryPool::ReleasePool()
{
	for(int i = 0; i < m_iPoolSize; i++)
	{
		if(pPOOL[i] != NULL)
		{
			delete pPOOL[i];
			pPOOL[i] = NULL;
		}
	}
}

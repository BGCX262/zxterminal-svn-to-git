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

//归还内存到内存池;
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
		//如果找不到，就删除内存空间;
		delete []pbuf;
	}
}

//得到空闲的内存;  没有空闲的内存就新申请
char* CMemoryPool::GetBuf()
{
    BOOL bFind = FALSE;
	for(int i = 0;i < m_iPoolSize; i ++)
	{
		if(pPOOL[i] != NULL)
		{
			//找到空闲的内存;
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

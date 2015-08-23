// IOCPBuff.cpp: implementation of the CIOCPBuff class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "IOCPBuff.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIOCPBuff::CIOCPBuff()
{

	Init();
}

CIOCPBuff::~CIOCPBuff()
{

}

void CIOCPBuff::Init()
{
	ZeroMemory(buf,sizeof(buf));
	ZeroMemory(&m_ol,sizeof(m_ol));
	m_UsedUnnber=0;
	m_Operation=-1;

}

void CIOCPBuff::SetupRead()
{
	if(m_UsedUnnber==0)
	{//如果没有使用任何的缓冲区
		this->m_wsabuf.buf=this->buf;
		this->m_wsabuf.len=MAXIOCPBUFSIZE;
	}
	else
	{//使用的话，就向后移动;
		m_wsabuf.buf = reinterpret_cast<char*>(buf) + m_UsedUnnber;
		m_wsabuf.len = MAXIOCPBUFSIZE - m_UsedUnnber; 
	}

}

void CIOCPBuff::EmptyUsed()
{
	this->m_UsedUnnber=0;
}

void CIOCPBuff::SetOpertion(int oper)
{
	this->m_Operation=oper;
}

void CIOCPBuff::SetSequenceNumber(int unm)
{
	this->m_iSequenceNumber=unm;

}

void CIOCPBuff::SetPosition(POSITION pos)
{
	this->m_pPos=pos;
}

POSITION CIOCPBuff::GetPosition()
{//得到位置;
	return  m_pPos;

}

int CIOCPBuff::Getoperation()
{
	return  this->m_Operation;
}

void CIOCPBuff::SetupZeroByteRead()
{	
	m_wsabuf.buf =(char*)buf;
	m_wsabuf.len = 0; 
}

WSABUF * CIOCPBuff::GetWSABuffer()
{
	return  const_cast<WSABUF*>(&m_wsabuf);
}

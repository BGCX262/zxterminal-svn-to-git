// EquimentList.cpp: implementation of the CEquimentList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "EquimentList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CEquimentList::CEquimentList()
{

}

CEquimentList::~CEquimentList()
{

}

EquiNetInfo  * CEquimentList::Find(CString szKey)
{
	
	m_lock.Lock();
	POSITION PosPre = NULL;
	POSITION Pos = this->GetHeadPosition();
	while( Pos )
	{
		PosPre = Pos;
		EquiNetInfo   *pEqui = (EquiNetInfo *)this->GetNext(Pos);
		if( pEqui  &&  pEqui->szKey==szKey)
		{
			m_lock.Unlock();
			return pEqui;
		}
	}
	m_lock.Unlock();
	return NULL;

}

void CEquimentList::AddUser(int fd, CString szKey,  struct sockaddr_in  addr  )
{	
	EquiNetInfo  *pEqui  = new EquiNetInfo;
	if(!pEqui)
	{
		return ;
	}
	pEqui->szKey=szKey;
	pEqui->fd=fd;
	pEqui->addr=addr;
	this->AddTail((void *)pEqui);
	return;


}



//edited 2006   
void CEquimentList::UpdatUser(int fd, CString szKey, sockaddr_in addr)
{
	EquiNetInfo*p=Find(szKey);
	m_lock.Lock();
	if(p)
	{//更新;
		p->fd=fd;
		memcpy(&p->addr,&addr,sizeof(addr));
	}
	else
	{//增加
		AddUser(fd,szKey,addr);	
	}
	m_lock.Unlock();
}

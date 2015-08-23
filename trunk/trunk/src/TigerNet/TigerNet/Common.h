#pragma once

#include "stdafx.h"

#include <string>
#include <deque>
//using namespace std;

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/circular_buffer.hpp>

#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/condition_variable.hpp>

#include <boost/filesystem.hpp>
//using namespace boost::filesystem;

#include <boost/exception/all.hpp>

#include <boost/crc.hpp>

//using namespace boost;
//using namespace boost::asio;

#include <boost/make_shared.hpp>
#include "boost/enable_shared_from_this.hpp"

#include "log4cpp/Category.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/BasicLayout.hh"
#include "log4cpp/PatternLayout.hh"

enum E_WN_PACKET_TYPE
{
	E_WN_TYPE_DATA  = 1,
	E_WN_TYPE_HB    = 2,
	E_WN_TYPE_LOGIN = 3
};

enum E_WN_COMPRESS_TYPE
{
	E_WN_COMPRESS_TYPE_NONE = 0,
	E_WN_COMPRESS_TYPE_ZLIB = 1,
	E_WN_COMPRESS_TYPE_OTHER = 2
};

class CPackHead
{
public:
	CPackHead(unsigned char btType,unsigned char btCompress,unsigned long ulLength)
	{
		m_btPrefix = 0x55;
		m_btVersion = 1;
		m_btType = btType;
		m_btCompress = btCompress;
		m_ulLength = ulLength;
	}

	~CPackHead()
	{

	}

	unsigned long size()
	{
		return 4 * sizeof(unsigned char) + sizeof(unsigned long);
	}

	unsigned char m_btPrefix;
	unsigned char m_btVersion;
	unsigned char m_btType;
	unsigned char m_btCompress;
	unsigned long m_ulLength;

};

class CPackBody
{
public:
	CPackBody(unsigned char btType,const unsigned char* pData,unsigned long ulLen)
	{
		if (ulLen != 0)
		{
			m_pBuffer = new unsigned char[ulLen];
			if (pData != NULL)
			{
				memcpy(m_pBuffer,pData,ulLen);
			}
			else
			{
				if (btType == 2)
				{
					memset(m_pBuffer, 0x11, 1);
				}
				else if (btType == 3)
				{
					memset(m_pBuffer, 0x22, 1);
				}
			}
			m_ulLength = ulLen;
		}
		else
		{
			m_pBuffer = NULL;
			m_ulLength = 0;
		}

	}

	~CPackBody()
	{
		if (m_pBuffer != NULL)
		{
			delete[] m_pBuffer;
			m_pBuffer = NULL;
		}

		m_ulLength = 0;
	}

	unsigned long size()
	{
		return m_ulLength * sizeof(unsigned char);
	}

	unsigned long m_ulLength;
	unsigned char* m_pBuffer;

};

class CPackTail
{
public:
	CPackTail()
	{
		m_btCheckSum = 0;
		m_btSuffix = 0x16;
	}

	~CPackTail()
	{

	}

	unsigned long size()
	{
		return 2 * sizeof(unsigned char);
	}

	unsigned char m_btCheckSum;
	unsigned char m_btSuffix;

};

class CPacket
{
public:

	CPacket(unsigned char btType,unsigned char btCompress,const unsigned char* pData, unsigned long ulLen):m_Head(btType,btCompress,ulLen),
		m_Body(btType,pData,ulLen)
	{
		m_ulLength = m_Head.size() + m_Body.size() + m_Tail.size();
		m_pBuffer = new unsigned char[m_ulLength];
		int nOffset = 0;
		memcpy(m_pBuffer + nOffset,&m_Head,m_Head.size());
		nOffset += m_Head.size();
		memcpy(m_pBuffer + nOffset,m_Body.m_pBuffer,m_Body.size());
		nOffset += m_Body.size();

		//checksum
		//crc32.reset();
		if (btType == 2)
		{
			m_btCheckSum = 0;
		} 
		else
		{
			crc32.process_bytes(m_Body.m_pBuffer,m_Body.size());
			m_btCheckSum = crc32();
		}
		m_Tail.m_btCheckSum = m_btCheckSum;
		
		///////////////////////

		memcpy(m_pBuffer + nOffset,&m_Tail,m_Tail.size());
		nOffset += m_Tail.size();

	}

	~CPacket()
	{
		if (m_pBuffer != NULL)
		{
			delete[] m_pBuffer;
			m_pBuffer = NULL;
		}

		m_ulLength = 0;
	}

	unsigned char* GetBuffer()
	{
		return m_pBuffer;
	}

	unsigned long GetSize()
	{
		return m_ulLength; 
	}


private:
	CPackHead m_Head;
	CPackBody m_Body;
	CPackTail m_Tail;

	boost::crc_32_type crc32;
	unsigned char m_btCheckSum;

	unsigned char* m_pBuffer;
	unsigned long m_ulLength;


};

class CNonMDFPacket
{
public:

	CNonMDFPacket(const unsigned char* pData,unsigned int unLen)
	{
		if (unLen != 0)
		{
			m_pBuffer = new unsigned char[unLen];
			if (pData != NULL)
			{
				memcpy(m_pBuffer,pData,unLen);
			}

			m_unLength = unLen;
		}
		else
		{
			m_pBuffer = NULL;
			m_unLength = 0;
		}

	}

	~CNonMDFPacket()
	{
		if (m_pBuffer != NULL)
		{
			delete[] m_pBuffer;
			m_pBuffer = NULL;
		}

		m_unLength = 0;
	}

	unsigned char* GetBuffer()
	{
		return m_pBuffer;
	}

	unsigned long GetSize()
	{
		return m_unLength; 
	}


private:
	unsigned char* m_pBuffer;
	unsigned int m_unLength;


};

void deleter(unsigned char* p);


typedef boost::shared_ptr<boost::asio::ip::tcp::socket> sock_pt;
typedef boost::shared_ptr<CPacket> packet_pt;
typedef boost::shared_ptr<CNonMDFPacket> nonmdfpacket_pt;

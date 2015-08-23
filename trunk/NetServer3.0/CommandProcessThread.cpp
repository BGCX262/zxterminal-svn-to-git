#include "stdafx.h"
#include "CommandProcessThread.h"


CCommandProcessThread::CCommandProcessThread(void)
{
}


CCommandProcessThread::~CCommandProcessThread(void)
{
}

void CCommandProcessThread::message(const _command & cmd)
{
	controlled_module_ex::message(cmd);
	if(cmd.nCmd == BM_USER + CMD_GPS_DATA)
	{
		std::cout << "get message" << std::endl;
		zaoxun::GPSData par = boost::any_cast<zaoxun::GPSData>(cmd.anyParam);
	}
}

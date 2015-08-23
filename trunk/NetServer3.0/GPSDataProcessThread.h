#pragma once

#include "controlled_module_ex.hpp"
#include "boost/any.hpp"
#include "msg.pb.h"

class CGPSDataProcessThread :
	public controlled_module_ex
{
public:
	CGPSDataProcessThread(void);
	~CGPSDataProcessThread(void);

protected:
	virtual void message(const _command & cmd);

private:
	void ProcessGPSData(zaoxun::CommonMessage msg);
	void UpdateNewestGPSData(GPSData gpsData);
};


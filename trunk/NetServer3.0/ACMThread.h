#pragma once
#include "controlled_module_ex.hpp"

#include "boost/any.hpp"
#include "msg.pb.h"

class ACMThread :
	public controlled_module_ex
{
public:
	ACMThread(void);
	~ACMThread(void);

protected:
	virtual void message(const _command & cmd);

private:
	bool IsMineCar(std::string strDeviceID);
	bool ACNCalculation(zaoxun::GPSData gpsData, zaoxun::AntiCollisionNotification& notification);
};


#include "stdafx.h"
#include "ACMThread.h"

#include "Common.h"
#include "MapXReplace.h"

extern std::map<std::string, zaoxun::GPSData> g_mapGPSData;
extern boost::mutex g_mapGPSData_mutex;

extern std::map<std::string, zxMineCar> g_mapMineCars;

ACMThread::ACMThread(void)
{
}


ACMThread::~ACMThread(void)
{
}

void ACMThread::message(const _command & cmd)
{
	controlled_module_ex::message(cmd);
	if(cmd.nCmd == BM_USER + CMD_GPS_DATA)
	{
		std::cout << "get message" << std::endl;
		zaoxun::GPSData par = boost::any_cast<zaoxun::GPSData>(cmd.anyParam);
	}
}

bool ACMThread::IsMineCar(std::string strDeviceID)
{
	std::map<std::string, zxMineCar>::iterator itr = g_mapMineCars.find(strDeviceID);
	if (itr != g_mapMineCars.end())
	{
		zxMineCar device = itr->second;
		if (device.nDeviceType = 1)
		{
			return false;
		} 
		else
		{
			return true;
		}
	}

	return false;
}

bool ACMThread::ACNCalculation(zaoxun::GPSData gpsData, zaoxun::AntiCollisionNotification& notification)
{
	boost::mutex::scoped_lock lock(g_mapGPSData_mutex);

	bool bResult = false;
	CMapXReplace MapXReplace;

	std::string strDeviceID = gpsData.device_id();
	double dLongitude = atof(gpsData.longitude().c_str());
	double dLatitude = atof(gpsData.latitude().c_str());

	std::map<std::string, zaoxun::GPSData>::iterator itr;
	for (itr = g_mapGPSData.begin(); itr != g_mapGPSData.end(); itr ++)
	{
		std::string strDevID = itr->second.device_id();
		if (!IsMineCar(strDevID) && strDeviceID != strDevID)
		{
			CMapXReplace::MapXPoint p1(dLongitude, dLatitude);
			CMapXReplace::MapXPoint p2(atof(itr->second.longitude().c_str()), atof(itr->second.latitude().c_str()));
			double dDistance = MapXReplace.CetDistance(p1, p2);
			if (dDistance < 100)
			{
				double dAngle = 0;
				notification.add_angle(dAngle);
				bResult = true;
			}
		}
		
	}

	return bResult;
}

#include "stdafx.h"
#include "NetServer.h"
#include "GPSDataProcessThread.h"

extern  CNetServerApp   theApp;
extern std::map<std::string, GPSData> g_mapGPSData;
extern boost::mutex g_mapGPSData_mutex;

CGPSDataProcessThread::CGPSDataProcessThread(void)
{
}


CGPSDataProcessThread::~CGPSDataProcessThread(void)
{
}

void CGPSDataProcessThread::message(const _command & cmd)
{
	controlled_module_ex::message(cmd);
	if(cmd.nCmd == BM_USER + CMD_GPS_DATA)
	{
		std::cout << "get message" << std::endl;
		zaoxun::CommonMessage msg = boost::any_cast<zaoxun::CommonMessage>(cmd.anyParam);
	}
}

void CGPSDataProcessThread::ProcessGPSData(zaoxun::CommonMessage msg)
{
	zaoxun::GPSData gpsData = msg.gps_data();

	theApp.m_pDBHelper->AddGPSData(gpsData);

	UpdateNewestGPSData(gpsData);

	HWND hWndRcv = ::FindWindow(NULL, "RMTServer");
	if (hWndRcv != NULL) 
	{
		CGPSData GpsData;
		strcpy(GpsData.szEquimentID, gpsData.device_id().c_str());
		GpsData.fLongitude = atof(gpsData.longitude().c_str());
		GpsData.fLatitude = atof(gpsData.latitude().c_str());
		GpsData.fSpeed = atof(gpsData.speed().c_str());
		GpsData.iDirection = atoi(gpsData.direction().c_str());
		GpsData.iState = 0;
		strcpy(GpsData.Time, gpsData.time().c_str());
		GpsData.iFlag = 0;

		COPYDATASTRUCT cpd;
		cpd.dwData = 1; // 标志为Student类型
		cpd.cbData = sizeof(CGPSData);
		cpd.lpData = (PVOID)&GpsData;

		::SendMessage(hWndRcv, WM_COPYDATA, (WPARAM)1, (LPARAM)&cpd);
	}
}

void CGPSDataProcessThread::UpdateNewestGPSData(GPSData gpsData)
{
	boost::mutex::scoped_lock lock(g_mapGPSData_mutex);

	std::map<std::string, zaoxun::GPSData>::iterator itr = g_mapGPSData.find(gpsData.device_id());
	if (itr != g_mapGPSData.end())
	{
		g_mapGPSData.erase(itr);
	} 
	g_mapGPSData.insert(std::pair<std::string, zaoxun::GPSData>(gpsData.device_id(), gpsData));
}

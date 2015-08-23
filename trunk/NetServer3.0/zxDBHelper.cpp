#include "stdafx.h"
#include "zxDBHelper.h"

#include "boost/regex.hpp"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <string>
#include <iostream>
#include <assert.h>
#include <algorithm>
#include <xfunctional>

#include "NetServer.h"

extern  CNetServerApp   theApp;

CzxDBHelper::~CzxDBHelper(void)
{
}


bool CzxDBHelper::Initialzie(std::string strDBHost, std::string strDBName, std::string strDBUser, std::string strDBPass)
{
	otl_connect::otl_initialize(1); 

	try
	{
		//string strConnectString = strDBUser + "/" + strDBPass + "@" + strDBHost + "/" + strDBName;
		std::string strConnectString = strDBUser + "/" + strDBPass + "@" + strDBName;
		m_otlDB->rlogon(strConnectString.c_str()); // connect to the database
	}
	catch(otl_exception& p)
	{ 
		// intercept OTL exceptions
		std::cerr << p.msg  << std::endl; // print out error message
		std::cerr << p.sqlstate << std::endl; // print out SQLSTATE 
		std::cerr << p.stm_text << std::endl; // print out SQL that caused the error
		std::cerr << p.var_info << std::endl; // print out the variable that caused the error

		//ACE_ERROR((LM_STARTUP, "%T TH:%t [%M] <%s> Database initialize error [%s] [%s] [%s] [%s]\n", __FUNCTION__,
		//	p.msg, p.sqlstate, p.stm_text, p.var_info));

		return false;
	}

	return true;
}

bool CzxDBHelper::GetDevice(std::string strDeviceID, zxDevice& device)
{
	std::string str = "select device_name, device_type, is_run from device_info where device_id = :device_id<char[15]> ";
	otl_stream query(1, str.c_str(), *theApp.m_otlDB);

	char szDeviceID[15];
	memset(szDeviceID, 0x00, 15);
	strcpy(szDeviceID, strDeviceID.c_str());

	query << szDeviceID;

	strcpy(device.szDeviceID, strDeviceID.c_str());

	if (!query.eof())
	{
		char szDeviceName[15];
		memset(szDeviceName, 0x00, 15);
		int nDeviceType;

		query >> device.szDeviceName;
		query >> device.nDeviceType;
		query >> device.nIsRun;
	}
	else
	{
		return false;
	}

	return true;
}

bool CzxDBHelper::GetMaterials(std::vector<zxMaterialInfo>& materials)
{
	std::string str = "select material_name from material";
	otl_stream query(1, str.c_str(), *theApp.m_otlDB);

	while (!query.eof())
	{
		zxMaterialInfo info;

		char szEnglishName[20];
		memset(szEnglishName, 0x00, 20);
		char szChineseName[20];
		memset(szChineseName, 0x00, 20);

		query >> szChineseName;

		strcpy(info.szChineseName, szChineseName);
		strcpy(info.szEnglishName, szEnglishName);

		materials.push_back(info);
	} 

	return true;
}

bool CzxDBHelper::GetEDippers(std::map<std::string, zxEdipper>& edippers)
{
	std::string str = "select device_id, device_name, device_type, is_run, halt_reason from device_info where device_type = 2";
	otl_stream query(1, str.c_str(), *theApp.m_otlDB);

	while(!query.eof())
	{
		char szDeviceID[15];
		memset(szDeviceID, 0x00, 15);

		char szDeviceName[15];
		memset(szDeviceName, 0x00, 15);

		int nDeviceType;
		int nIsRun;
		int nHaltReason;

		query >> szDeviceID;
		query >> szDeviceName;
		query >> nDeviceType;
		query >> nIsRun;
		query >> nHaltReason;

		std::string str = "select longitude, latitude  from gps_current where device_id = :device_id<char[15]>";
		otl_stream queryPosition(1, str.c_str(), *theApp.m_otlDB);

		queryPosition << szDeviceID;

		double dLongitude = 0;
		double dLatitude = 0;

		if (!queryPosition.eof())
		{
			queryPosition >> dLongitude;
			queryPosition >> dLatitude;
		}
		
		zxEdipper info;
		strcpy(info.szDeviceID, szDeviceID);
		strcpy(info.szDeviceName, szDeviceName);
		info.nDeviceType = nDeviceType;
		info.nIsRun = nIsRun;
		info.nHaltReason = nHaltReason;
		info.dLongitude = dLongitude;
		info.dLatitude = dLatitude;

		edippers.insert(std::pair<std::string, zxEdipper>(szDeviceID, info));
	} 

	return true;
}

bool CzxDBHelper::GetMineCars(std::map<std::string, zxMineCar>& minecars)
{
	std::string str = "select device_id, device_name, device_type, is_run, halt_reason from device_info where device_type = 1";
	otl_stream query(1, str.c_str(), *theApp.m_otlDB);

	while(!query.eof())
	{
		char szDeviceID[15];
		memset(szDeviceID, 0x00, 15);

		char szDeviceName[15];
		memset(szDeviceName, 0x00, 15);

		int nDeviceType;
		int nIsRun;
		int nHaltReason;

		query >> szDeviceID;
		query >> szDeviceName;
		query >> nDeviceType;
		query >> nIsRun;
		query >> nHaltReason;
		
		zxMineCar info;
		strcpy(info.szDeviceID, szDeviceID);
		strcpy(info.szDeviceName, szDeviceName);
		info.nDeviceType = nDeviceType;
		info.nIsRun = nIsRun;
		info.nHaltReason = nHaltReason;

		minecars.insert(std::pair<std::string, zxMineCar>(szDeviceID, info));
	} 

	return true;
}


std::vector<std::string> split(std::string str, std::string s)
{
	boost::regex reg(s.c_str());
	std::vector<std::string> vec;
	boost::sregex_token_iterator it(str.begin(),str.end(),reg,-1);
	boost::sregex_token_iterator end;
	while(it != end)
	{
		vec.push_back(*it++);
	}

	return vec;
}

void GetPostionsFromString(char* szPositions, std::vector<zxPosition>& vctPositions)
{
	std::string strPositions = szPositions;
	//std::string::iterator new_end1 = std::remove_if(strPositions.begin(), strPositions.end(), std::bind2nd(std::equal_to<char>(), '['));
	//strPositions.erase(new_end1, strPositions.end());

	//std::string::iterator new_end2 = std::remove_if(strPositions.begin(), strPositions.end(), std::bind2nd(std::equal_to<char>(), ']'));
	//strPositions.erase(new_end2, strPositions.end());

	//std::vector<std::string> vecs = split(strPositions, ",");
	std::vector<std::string> vStr;
	boost::split( vStr, strPositions, boost::is_any_of( ";" ), boost::token_compress_on);
	for( std::vector<std::string>::iterator it = vStr.begin(); it != vStr.end(); ++ it )
	{
		std::string str = *it;
		std::string::iterator new_end1 = std::remove_if(str.begin(), str.end(), std::bind2nd(std::equal_to<char>(), '('));
		str.erase(new_end1, str.end());

		std::string::iterator new_end2 = std::remove_if(str.begin(), str.end(), std::bind2nd(std::equal_to<char>(), ')'));
		str.erase(new_end2, str.end());

		int nIdx = str.find(',');
		if (nIdx != std::string::npos)
		{
			std::string strLongitude = str.substr(0, nIdx);
			std::string strLatitude = str.substr(nIdx + 1);

			zxPosition position;
			position.dLongitude = atof(strLongitude.c_str());
			position.dLatitude = atof(strLatitude.c_str());

			vctPositions.push_back(position);
		}
	}
}

bool CzxDBHelper::GetAreas(std::map<int, CzxArea>& areas)
{
	std::string str = "select area_id, area_name, update_time, area_position, sunny_limited_speed, rainy_limited_speed, is_run, map_id, traffic_count from area";
	otl_stream query(1, str.c_str(), *theApp.m_otlDB);

	while(!query.eof())
	{
		char szPositions[256];
		memset(szPositions, 0x00, 256);

		CzxArea area;

		otl_long_string str;

		query >> area.m_nAreaID;
		query >> area.m_szAreaName;
		query >> area.m_szUpdateTime;

		query >> str;
		memcpy(szPositions, str.v, str.len());
		GetPostionsFromString(szPositions, area.m_vctAreaPositions);

		query >> area.m_dSunnyLimitedSpeed;
		query >> area.m_dRainyLimitedSpeed;
		query >> area.m_nIsRun;
		query >> area.m_nMapID;
		query >> area.m_nTrafficCount;

		areas.insert(std::pair<int, CzxArea>(area.m_nAreaID, area));
	} 

	return true;
}

bool CzxDBHelper::GetDriver(int nDriverID, zxDriver& driver)
{
	std::string str = "select driver_id, driver_name from driver_info where driver_id = :driver_id<int> ";
	otl_stream query(1, str.c_str(), *theApp.m_otlDB);

	query << nDriverID;

	if (!query.eof())
	{
		query >> driver.nDriverID;
		query >> driver.szDriverName;
	}
	else
	{
		return false;
	}

	return true;
}

bool CzxDBHelper::GetDriver(std::string strDeviceID, zxDriver& driver)
{
	zxClassTime classTime;
	memset(&classTime, 0x00, sizeof(zxClassTime));
	GetClassTime(classTime);

	char pastYear[8] = {0}; //前一天所处的年份
	char currYear[8] = {0}; //当前所处的年份

	char pastMonth[8] = {0}; //前一天所处的月份
	char currMonth[8] = {0}; //当前所处的月份

	char pastDay[8] = {0}; //前一天所处的日期
	char currDay[8] = {0}; //当前所处的日期

	time_t	timePast, timeCurr;
	struct tm *timePastStru, *timeCurrStru;

	timeCurr = time(NULL);
	timeCurrStru = localtime(&timeCurr);

	CString currentTime;
	currentTime.Format("%s-%s-%s %02d:%02d:%02d", currYear, currMonth, currDay, timeCurrStru->tm_hour, timeCurrStru->tm_min, timeCurrStru->tm_sec);

	CString currentNightBeginTime;
	currentTime.Format("%s-%s-%s %s", currYear, currMonth, currDay, classTime.szNightBeginTime);

	int nShiftID = 0;

	char szDate[32];
	memset(szDate, 0x00, 32);

	if (currentTime < currentNightBeginTime)
	{
		CString currentDate;
		currentDate.Format("%s-%s-%s", currYear, currMonth, currDay);

		timePast = time(NULL) - 24 * 3600;
		timePastStru = localtime(&timePast);

		strftime(pastYear, 8, "%Y", timePastStru);
		strftime(pastMonth, 8, "%m", timePastStru);
		strftime(pastDay, 8, "%d", timePastStru);

		strftime(currYear, 8,"%Y", timeCurrStru);
		strftime(currMonth, 8,"%m", timeCurrStru);
		strftime(currDay, 8, "%d", timeCurrStru);

		CString strMorningBeginTime, strNoonBeginTime, strNightBeginTime, strAllBeginTime, strAllEndTime, strCurrTaskRecordTable;
		strMorningBeginTime = classTime.szMorningBeginTime;
		strNoonBeginTime = classTime.szNoonBeginTime;
		strNightBeginTime = classTime.szNightBeginTime;

		strAllBeginTime.Format("%s-%s-%s %s:00", pastYear, pastMonth, pastDay, strNightBeginTime);
		strAllEndTime.Format("%s-%s-%s %s:00", currYear, currMonth, currDay, strNightBeginTime);

		CString strMorningStart, strMorningEnd, strNoonStart, strNoonEnd, strNightStart, strNightEnd;

		strNightStart.Format("%s-%s-%s %s:00", pastYear, pastMonth, pastDay, strNightBeginTime.GetBuffer(0));
		strNightEnd.Format("%s-%s-%s %s:00", currYear, currMonth, currDay, strMorningBeginTime.GetBuffer(0));

		strMorningStart.Format("%s-%s-%s %s:00", currYear, currMonth, currDay, strMorningBeginTime.GetBuffer(0));
		strMorningEnd.Format("%s-%s-%s %s:00", currYear, currMonth, currDay, strNoonBeginTime.GetBuffer(0));
		strNoonStart.Format("%s-%s-%s %s:00", currYear, currMonth, currDay, strNoonBeginTime.GetBuffer(0));
		strNoonEnd.Format("%s-%s-%s %s:00", currYear, currMonth, currDay, strNightBeginTime.GetBuffer(0));
		
		if (currentTime >= strNightStart && currentTime <= strNightEnd)
		{
			nShiftID = 1;
		} 
		else if (currentTime >= strMorningStart && currentTime <= strMorningEnd)
		{
			nShiftID = 2;
		}
		else
		{
			nShiftID = 3;
		}

		strcpy(szDate, currentDate.GetBuffer(0));
	} 
	else
	{
		time_t	timeNext;
		struct tm* timeNextStru;

		timeNext = time(NULL) + 24 * 3600;
		timeNextStru = localtime(&timeNext);

		CString nextDate;
		nextDate.Format("%s-%02d-%02d", timeNextStru->tm_year + 1900, timeNextStru->tm_mon + 1, timeNextStru->tm_mday);

		nShiftID = 1;
		strcpy(szDate, nextDate.GetBuffer(0));
	}

	std::string str = "select driver_id from schedule_record where device_id = :device_id<char[32]> and date = :date<char[16]> and shift_id = :shift_id<int>";
	otl_stream query(1, str.c_str(), *theApp.m_otlDB);

	char szDeviceID[32];
	memset(szDeviceID, 0x00, 32);
	strcpy(szDeviceID, strDeviceID.c_str());

	query << szDeviceID;
	query << szDate;
	query << nShiftID;

	if (!query.eof())
	{
		query >> driver.nDriverID;
		GetDriver(driver.nDriverID, driver);
	}
	else
	{
		return false;
	}

	return true;
}

bool CzxDBHelper::GetClassTime(zxClassTime& classTime)
{
	std::string str = "select morning_begin_time, noon_begin_time, night_begin_time, weather, safty_distance from parameter";
	otl_stream query(1, str.c_str(), *theApp.m_otlDB);

	if (!query.eof())
	{
		query >> classTime.szMorningBeginTime;
		query >> classTime.szNoonBeginTime;
		query >> classTime.szNightBeginTime;
		query >> classTime.nWeather;
		query >> classTime.szSafeDistance;
	}
	else
	{
		return false;
	}

	return true;
}

bool CzxDBHelper::CreateTable(std::string strTableName)
{
	//string str = "show tables like '" + strTableName + "'";
	std::string str = "select `TABLE_NAME` from `INFORMATION_SCHEMA`.`TABLES` where `TABLE_SCHEMA`='netdb' and `TABLE_NAME`='" + strTableName + "'";
	otl_stream query(1, str.c_str(), *theApp.m_otlDB);

	if (query.eof())
	{
		std::string str1 = "create table " + strTableName;
		str1 += "(device_id varchar(15), time varchar(20) , longitude double, latitude double, elevation double, direction int, speed double)"; 
		otl_stream query(1, str1.c_str(), *theApp.m_otlDB);
	}

	return true;
}

bool CzxDBHelper::AddGPSData(GPSData gpsData)
{
	try
	{
		//add record to gps current table
		std::string str = "select * from gps_current where device_id = :device_id<char[15]>";
		otl_stream query(1, str.c_str(), *theApp.m_otlDB);

		char szDeviceID[15];
		memset(szDeviceID, 0x00, 15);
		strcpy(szDeviceID, gpsData.device_id().c_str());
		query << szDeviceID;

		if (!query.eof())
		{
			//update data
			std::string str1 = "update gps_current set time = :v_time<char[20]>, longitude = :v_longitude<double>, latitude = :v_latitude<double>, elevation = :v_elevation<double>, direction = :v_direction<int>, speed = :v_speed<double> where device_id = :v_device_id<char[15]>";
			otl_stream query(1, str1.c_str(), *theApp.m_otlDB);

			char szDeviceID[15];
			char szTime[20];
			memset(szDeviceID, 0x00, 15);
			memset(szTime, 0x00, 20);
			strcpy(szDeviceID, gpsData.device_id().c_str());
			strcpy(szTime, gpsData.time().c_str());

			query << szTime;
			query << atof(gpsData.longitude().c_str());
			query << atof(gpsData.latitude().c_str());
			query << atof(gpsData.elevation().c_str());
			query << atoi(gpsData.direction().c_str());
			query << atof(gpsData.speed().c_str());
			query << szDeviceID;
		} 
		else
		{
			//insert data
			std::string str2 = "insert into gps_current values";
			str2 += "(:device_id<char[15]>, :time<char[20]>, :longitude<double>, :latitude<double>, :elevation<double>, :direction<int>, :speed<double>)";
			otl_stream query(1, str2.c_str(), *theApp.m_otlDB);

			char szDeviceID[15];
			char szTime[20];
			memset(szDeviceID, 0x00, 15);
			memset(szTime, 0x00, 20);
			strcpy(szDeviceID, gpsData.device_id().c_str());
			strcpy(szTime, gpsData.time().c_str());

			query << szDeviceID;
			query << szTime;
			query << atof(gpsData.longitude().c_str());
			query << atof(gpsData.latitude().c_str());
			query << atof(gpsData.elevation().c_str());
			query << atoi(gpsData.direction().c_str());
			query << atof(gpsData.speed().c_str());
		}


		//add record to gps daily table
		char szTableName[20];
		memset(szTableName, 0x00, 20);
		COleDateTime current = COleDateTime::GetCurrentTime();
		sprintf(szTableName, "%s", current.Format("gps_%Y%m%d"));

		CreateTable(szTableName);

		std::string strTableName = szTableName;
		std::string str3 = "insert into " + strTableName + " values";
		str3 += "(:device_id<char[15]>, :time<char[20]>, :longitude<double>, :latitude<double>, :elevation<double>, :direction<int>, :speed<double>)";
		otl_stream insertQuery(1, str3.c_str(), *theApp.m_otlDB);

		//char szDeviceID[15];
		char szTime[20];
		memset(szDeviceID, 0x00, 15);
		memset(szTime, 0x00, 20);
		strcpy(szDeviceID, gpsData.device_id().c_str());
		strcpy(szTime, gpsData.time().c_str());

		insertQuery << szDeviceID;
		insertQuery << szTime;
		insertQuery << atof(gpsData.longitude().c_str());
		insertQuery << atof(gpsData.latitude().c_str());
		insertQuery << atof(gpsData.elevation().c_str());
		insertQuery << atoi(gpsData.direction().c_str());
		insertQuery << atof(gpsData.speed().c_str());

		return true;
	}
	catch (const otl_exception& e)
	{
		return false;
	}
	catch (const std::exception& e)
	{
		return false;
	}
}

bool CzxDBHelper::GetCurrentArea(std::string strDeviceID, zxArea& area)
{
	try
	{
		std::string str = "select area_id from gps_current where device_id = :device_id<char[15]>";
		otl_stream query(1, str.c_str(), *theApp.m_otlDB);

		char szDeviceID[15];
		memset(szDeviceID, 0x00, 15);
		strcpy(szDeviceID, strDeviceID.c_str());
		query << szDeviceID;

		if (!query.eof())
		{
			query >> area.nAreaID;
		}
	}
	catch (const otl_exception& e)
	{
		return false;
	}
	catch (const std::exception& e)
	{
		return false;
	}
	
	return true;
}
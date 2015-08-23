#pragma once

#include "boost/noncopyable.hpp"

#define OTL_ODBC_MYSQL
#include "otlv4.h"

#include <iostream>
#include <string>
#include <vector>

#include "zxArea.h"

#include "msg.pb.h"
using namespace zaoxun;

enum 
{
	E_RC_DB_SUCCESS = 0,
	E_RC_DB_FAILURE = -1,
	E_RC_DB_ITEM_ALREADY_EXISTS = -2,	//已存在
	E_RC_DB_ITEM_NOT_EXIST = -3,		//不存在
	E_RC_DB_ITEM_USER_PSW_ERROR = -4,	//用户密码错误
};

class CzxDBHelper
{
public:
	CzxDBHelper(otl_connect* otldb) : m_otlDB(otldb)
	{
	}

	~CzxDBHelper(void);

	bool Initialzie(std::string strDBHost, std::string strDBName, std::string strDBUser, std::string strDBPass);

	bool GetDevice(std::string strDeviceID, zxDevice& device);

	bool GetMaterials(std::vector<zxMaterialInfo>& materials);

	bool GetEDippers(std::map<std::string, zxEdipper>& edippers);

	bool GetMineCars(std::map<std::string, zxMineCar>& minecars);

	bool GetAreas(std::map<int, CzxArea>& areas);

	bool GetDriver(int nDriverID, zxDriver& driver);

	bool GetDriver(std::string strDeviceID, zxDriver& driver);

	bool GetClassTime(zxClassTime& classTime);

	bool CreateTable(std::string strTableName);

	bool AddGPSData(GPSData gpsData);

	bool GetCurrentArea(std::string strDeviceID, zxArea& area);

public:
	otl_connect* m_otlDB;
};


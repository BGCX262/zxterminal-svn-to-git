#pragma once

#include "msg.pb.h"
using namespace zaoxun;

typedef struct 
{
	char szEnglishName[20];
	char szChineseName[20];
}zxMaterialInfo;

typedef struct 
{
	char szDeviceID[15];
	double dLongitude;
	double dLatitude;
}zxEDipperInfo;

typedef struct 
{
	double dLongitude;
	double dLatitude;
}zxPosition;

typedef struct  
{
	char szDeviceID[15];
	char szDeviceName[15];
	int  nDeviceType;
	int  nIsRun;
}zxDevice;

typedef struct  
{
	char szDeviceID[15];
	char szDeviceName[15];
	int  nDeviceType;
	int  nIsRun;
	int  nHaltReason;
	double dLongitude;
	double dLatitude;
}zxEdipper;

typedef struct  
{
	char szDeviceID[15];
	char szDeviceName[15];
	int  nDeviceType;
	int  nIsRun;
	int  nHaltReason;
}zxMineCar;

typedef struct  
{
	char szDeviceID[32];
	char szDeviceName[32];
	int  nMaterialID;
	char szDriverID[32];
	char szDriverName[32];
	char szEdipperID[32];
	char szEdipperName[32];
	char szEdipperDriverID[64];
	char szEdipperDriverName[64];
	int  nLoadingAreaID;
	char szLoadingAreaName[32];
	int  nDumpingAreaID;
	char szDumpingAreaName[32];
	char szLoadingTime[32];
	char szDumpingTime[32];
	double dDistance;
	char szTargetDeviceID[32];
	char szTargetDeviceName[32];
}zxTaskRecord;

typedef struct  
{
	int  nAreaID;
	char szAreaName[32];
}zxArea;

typedef struct  
{
	int  nDriverID;
	char szDriverName[32];
}zxDriver;

typedef struct  
{
	char szMorningBeginTime[16];
	char szNoonBeginTime[16];
	char szNightBeginTime[16];
	int nWeather;
	char szSafeDistance[32];
}zxClassTime;

typedef struct  
{
	char szDeviceID[16];
	char szDeviceName[16];
	int  nType;
	int  nFD;
	char szLoginTime[20];
}zxClient;
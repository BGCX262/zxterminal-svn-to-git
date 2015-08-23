#pragma once

#include "Common.h"

//#include	"../../Include/log4cpp/Category.hh"
//#include	"../../Include/log4cpp/FileAppender.hh"
//#include	"../../Include/log4cpp/BasicLayout.hh"


//#define LOG_PRIORITY_FATAL   log4cpp::Priority::FATAL
//#define LOG_PRIORITY_ALERT   log4cpp::Priority::ALERT
//#define LOG_PRIORITY_CRIT    log4cpp::Priority::CRIT
//#define LOG_PRIORITY_ERROR   log4cpp::Priority::ERROR
//#define LOG_PRIORITY_WARN    log4cpp::Priority::WARN
//#define LOG_PRIORITY_NOTICE  log4cpp::Priority::NOTICE
//#define LOG_PRIORITY_INFO    log4cpp::Priority::INFO
//#define LOG_PRIORITY_DEBUG   log4cpp::Priority::DEBUG
//#define LOG_PRIORITY_NOTSET  log4cpp::Priority::NOTSET


enum WnLogLevelValue
{
	WN_LOG_LEVEL_DEBUG = log4cpp::Priority::DEBUG,
	WN_LOG_LEVEL_INFO  = log4cpp::Priority::INFO,
	WN_LOG_LEVEL_WARN  = log4cpp::Priority::WARN,
	WN_LOG_LEVEL_ERROR = log4cpp::Priority::ERROR,
	WN_LOG_LEVEL_FATAL = log4cpp::Priority::FATAL,
};




class CLogWriter
{
public:
	CLogWriter(void);
	~CLogWriter(void);

	//初始化
#ifdef _DEBUG
	bool Initialize(int nLogLevel = 3,char* szFileName = NULL);
#else
	bool Initialize(int nLogLevel = 3,char* szFileName = NULL);
#endif
	//设置日志级别
	void SetLogLevel(WnLogLevelValue level);
	//void Log(char* szMessage);
	static void Log(WnLogLevelValue level,char* szMessage);
	void Uninitialize();
	

	bool FolderExists(char* s);
	bool CreateDir(char* P);

public:
	char m_szFileName[256];
	WnLogLevelValue m_LogLevel;
	static log4cpp::Category& m_LogCate;
	//log4cpp::Category& m_LogCate;
	static int m_nSeq ;
	static std::string m_strCatName;
	//static string m_str = "1";

};



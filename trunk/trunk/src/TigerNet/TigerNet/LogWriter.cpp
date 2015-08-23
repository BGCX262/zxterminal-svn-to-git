#include "stdafx.h"
#include "LogWriter.h"

int CLogWriter::m_nSeq = 1;
std::string CLogWriter::m_strCatName = std::string("CommonServer_") + boost::lexical_cast<std::string>(m_nSeq ++);
log4cpp::Category& CLogWriter::m_LogCate = log4cpp::Category::getInstance(CLogWriter::m_strCatName.c_str());

CLogWriter::CLogWriter(void)// : m_strCatName(string("CommonServer_") + boost::lexical_cast<string>(m_nSeq ++))
	//m_LogCate( log4cpp::Category::getInstance(m_strCatName.c_str()) )
{
	//char szCategoryName[256];
	//sprintf(szCategoryName, "CommonServer%d", m_nSeq);
	//m_LogCate = log4cpp::Category::getInstance(szCategoryName);
	// 4. 实例化一个category对象
	//log4cpp::Category log = log4cpp::Category::getInstance("SpeedMonitor");
	//m_LogCate = log4cpp::Category::getRoot();
	//log4cpp::Category::getInstance("SpeedMonitor");
}

CLogWriter::~CLogWriter(void)
{
}

//判断目录是否存在  
bool CLogWriter::FolderExists(char* szFolder)    
{    
	//DWORD attr;     
	//attr = GetFileAttributes(s);     
	//return (attr != (DWORD)(-1) ) && ( attr & FILE_ATTRIBUTE_DIRECTORY);     

	boost::filesystem::path folder(szFolder);
	if (boost::filesystem::exists(folder))
	{
		return true;
	} 
	else
	{
		return false;
	}
	
}    

//创建目录
bool CLogWriter::CreateDir(char* szDirectory)    
{    
	boost::filesystem::path dir(szDirectory);
	return create_directory(dir);
}    


bool CLogWriter::Initialize(int nLogLevel,char* szFileName)
{
	//获取EXE全路径
	std::string strExePath = boost::filesystem::initial_path<boost::filesystem::path>().string();

	WnLogLevelValue level;
	switch(nLogLevel)
	{
	case 0:
		level = WN_LOG_LEVEL_DEBUG;
		break;
	case 1:
		level = WN_LOG_LEVEL_INFO;
		break;
	case 2:
		level = WN_LOG_LEVEL_WARN;
		break;
	case 3:
		level = WN_LOG_LEVEL_ERROR;
		break;
	case 4:
		level = WN_LOG_LEVEL_FATAL;
		break;
	default:
		level = WN_LOG_LEVEL_DEBUG;
		break;
	}

	m_LogLevel = level;

	char szLogFileName[256];
	if (szFileName == NULL || strlen(szFileName) == 0)
	{
		sprintf(szLogFileName,"%s.log", m_strCatName.c_str());
	}
	else
	{
		strcpy(szLogFileName, szFileName);
	}

	//CreateDir(strExePath.c_str());

	sprintf(m_szFileName,"%s/%s",strExePath.c_str(), szLogFileName);

	// 1实例化一个layout 对象
	log4cpp::PatternLayout* layout = new log4cpp::PatternLayout();
	layout->setConversionPattern("[%d] [%p] [%t] %m %n");  //[%c] 


	// 2. 初始化一个appender 对象
	log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender",	m_szFileName);
	// 3. 把layout对象附着在appender对象上
	appender->setLayout(layout);
	// 4. 实例化一个category对象
	//log4cpp::Category& warn_log = log4cpp::Category::getInstance("mywarn");
	//m_LogCate = log4cpp::Category::getInstance(szLogFileName);//"SpeedMonitor"
	// 5. 设置additivity为false，替换已有的appender
	m_LogCate.setAdditivity(false);
	// 5. 把appender对象附到category上
	m_LogCate.setAppender(appender);
	// 6. 设置category的优先级，低于此优先级的日志不被记录
	m_LogCate.setPriority(m_LogLevel);//log4cpp::Priority::INFO
	// 记录一些日志
	//m_LogCate.info("Program info which cannot be wirten");
	//m_LogCate.debug("This debug message will fail to write");
	//m_LogCate.alert("Alert info");
	// 其他记录日志方式
	//m_LogCate.log(log4cpp::Priority::DEBUG, "This will be a debug logging");
	//m_LogCate.log(log4cpp::Priority::INFO,  "This will be a info  logging");
	//m_LogCate.log(log4cpp::Priority::WARN,  "This will be a warn  logging");
	//m_LogCate.log(log4cpp::Priority::ERROR, "This will be a error logging");
	//m_LogCate.log(log4cpp::Priority::FATAL, "This will be a fatal logging");

	return true;

}

void CLogWriter::SetLogLevel(WnLogLevelValue level)
{
	m_LogLevel = level;
}

void CLogWriter::Log(WnLogLevelValue level,char* szMessage)
{
	m_LogCate.log(level, szMessage);
}

void CLogWriter::Uninitialize()
{
	m_LogCate.shutdown();
}

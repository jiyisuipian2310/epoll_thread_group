#include "ConfigInit.h"
#include "FileReader.h"
#include <cstdarg>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <errno.h>
#include "log.h"

extern LogLevel g_logLevel;

CConfig* CConfig::instance() {
    static CConfig _instance;
    return &_instance;
}

bool CConfig::init( const char * conf )
{
    char errmsg[256] = {0};
    CFileReader iniFileReader;
    if(!iniFileReader.ReadFile(conf)) {
        sprintf(errmsg, "Error: open config file %s failed, reason: %s", conf, strerror(errno));
        cout << errmsg << endl;
        return false;
    }

    m_strListenIp = iniFileReader.Get_Profile_Str("MAIN","LISTEN_IP","127.0.0.1");
    m_strListenPort = iniFileReader.Get_Profile_Str("MAIN","LISTEN_PORT", "9999");

    m_nThreadCount = iniFileReader.Get_Profile_Int("MAIN","THREAD_COUNT", 16);
    m_nTimeoutDisconnect = iniFileReader.Get_Profile_Int("MAIN","TIMEOUT_DISCONNECT", 1800);
    
    m_strLogPath = iniFileReader.Get_Profile_Str("LOG","LOG_PATH", "./log");
    m_strLogOutput = iniFileReader.Get_Profile_Str("LOG","LOG_OUTPUT", "file");

    if(m_strLogOutput != "console") {
        std::tm local_time;
        std::time_t now = std::time(0);
        localtime_r(&now, &local_time);

        char buffer[32] = { 0 };
        strftime(buffer, sizeof(buffer), "%Y%m%d", &local_time);
        string strLogName = m_strLogPath + "/server_" + buffer + ".log";
        pLogFile = fopen(strLogName.data(), "a+");
        if(pLogFile == NULL) {
            sprintf(errmsg, "Error: open log file %s failed, reason: %s", strLogName.data(), strerror(errno));
            cout << errmsg << endl;
            return false;
        }
    }

    m_strLogLevel = iniFileReader.Get_Profile_Str("LOG","LOG_LEVEL", "INFO");
    if("TRACE" == m_strLogLevel) {
        g_logLevel = LOG_LEVEL_TRACE;
    } else if("DEBUG" == m_strLogLevel) {
        g_logLevel = LOG_LEVEL_DEBUG;
    } else if("INFO" == m_strLogLevel) {
        g_logLevel = LOG_LEVEL_INFO;
    } else if("WARN" == m_strLogLevel) {
        g_logLevel = LOG_LEVEL_WARN;
    } else if("ERROR" == m_strLogLevel) {
        g_logLevel = LOG_LEVEL_ERROR;
    } else {
        g_logLevel = LOG_LEVEL_INFO;
    }

    LOG_LOG(LOG_LEVEL_FINAL, "FINAL", "MAIN.LOG_LEVEL: %s", m_strLogLevel.c_str());
    LOG_LOG(LOG_LEVEL_FINAL, "FINAL", "MAIN.LISTEN_IP: %s", m_strListenIp.c_str());
    LOG_LOG(LOG_LEVEL_FINAL, "FINAL", "MAIN.LISTEN_PORT: %s", m_strListenPort.c_str());
    LOG_LOG(LOG_LEVEL_FINAL, "FINAL", "MAIN.THREAD_COUNT: %d", m_nThreadCount);
    LOG_LOG(LOG_LEVEL_FINAL, "FINAL", "MAIN.TIMEOUT_DISCONNECT: %d", m_nTimeoutDisconnect);

    LOG_LOG(LOG_LEVEL_FINAL, "FINAL", "LOG.LOG_PATH: %s", m_strLogPath.c_str());
    LOG_LOG(LOG_LEVEL_FINAL, "FINAL", "LOG.LOG_OUTPUT: %s", m_strLogOutput.c_str());

    return true;
}
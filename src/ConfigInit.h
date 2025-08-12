#ifndef __CONFIG_INIT_H__
#define __CONFIG_INIT_H__

#include <string>
#include <stdint.h>
#include <vector>

using namespace std;

class CConfig {
public:
	static CConfig* instance();
	bool init( const char * conf );

public:
	string m_strListenIp;
    string m_strListenPort;
    string m_strLogLevel;
    string m_strLogPath;
    string m_strLogOutput;
    FILE* pLogFile;

public:
    int m_nThreadCount; // default thread count
    int m_nTimeoutDisconnect; // default timeout disconnect
};

#endif // __CONFIG_INIT_H__
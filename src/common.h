#ifndef __COMMON_H__
#define __COMMON_H__

#include <iostream>
#include <string>
#include <set>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <atomic>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <ctime>
#include <libgen.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <sstream>

using namespace std;

class CConfig;
class ReactorThread;
class AppManager;

struct stAppConfig {
public:
    stAppConfig() {
        pReactorThread = NULL;
        nExitThreadCount = 0;
        pConfig = NULL;
        strLastDate = "";
        bMainThreadExit = false;
        pAppManager = NULL;
        nRunThreadCount = 0;
    }

public:
    AppManager* pAppManager;
    std::atomic<int> nExitThreadCount;
    int nRunThreadCount;
    ReactorThread* pReactorThread;
    CConfig* pConfig;
    string strLastDate;
    bool bMainThreadExit;
};

string get_current_time(string format);
void check_app_running();
set<int> parse_listen_ports(const string& portStr);

#endif //__COMMON_H__
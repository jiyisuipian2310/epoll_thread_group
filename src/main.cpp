#include "EpollReactor.h"
#include "ReactorThread.h"
#include "AcceptorEvent.h"
#include "log.h"
#include "AppManager.h"
#include "ConfigInit.h"
#include <signal.h>

#define cfg_file "./HostInfo.cfg"

LogLockFreeQueue g_logger;
LogLevel g_logLevel;
stAppConfig g_appConfig;

void signal_callback(int signo)
{
    if(signo != SIGPIPE) {
        g_appConfig.pReactorThread->stop_thread();
        g_appConfig.pAppManager->stop_thread();
        g_appConfig.bMainThreadExit = true;
    }
}

int main(int argc, char** argv)
{
    check_app_running();

    string errmsg;
    string strTotalErrMsg = get_current_time("[%Y%m%d %H:%M:%S][ERROR][main]: ");
    g_appConfig.strLastDate = get_current_time("%Y%m%d");

    g_appConfig.pConfig = CConfig::instance();
    if(!g_appConfig.pConfig->init(cfg_file)) {
        return -1;
    }

    /************************************************************************/
    EpollReactor* pEpollReactor = new EpollReactor();
    if(!pEpollReactor->init(errmsg)) {
        strTotalErrMsg.append(errmsg).append("\n");
        fwrite(strTotalErrMsg.data(), strTotalErrMsg.size(), 1, g_appConfig.pConfig->pLogFile);
        return -1;
    }

    g_appConfig.pReactorThread = ReactorThread::instance();
    if(!g_appConfig.pReactorThread->init(pEpollReactor, errmsg)) {
        strTotalErrMsg.append(errmsg).append("\n");
        fwrite(strTotalErrMsg.data(), strTotalErrMsg.size(), 1, g_appConfig.pConfig->pLogFile);
        return -1;
    }

    g_appConfig.pReactorThread->start_thread();
    /************************************************************************/
    g_appConfig.pAppManager = AppManager::instance();
    if(!g_appConfig.pAppManager->init(errmsg)) {
        strTotalErrMsg.append(errmsg).append("\n");
        fwrite(strTotalErrMsg.data(), strTotalErrMsg.size(), 1, g_appConfig.pConfig->pLogFile);
        return -1;
    }

    vector<AcceptorEvent*> acceptors;
    set<int> ports = parse_listen_ports(g_appConfig.pConfig->m_strListenPort);
    for (int port : ports) {
        AcceptorEvent* pAcceptorEvent = new AcceptorEvent();
        if(false == pAcceptorEvent->open_tcp(g_appConfig.pConfig->m_strListenIp, port, errmsg)) {
            strTotalErrMsg.append(errmsg).append("\n");
            fwrite(strTotalErrMsg.data(), strTotalErrMsg.size(), 1, g_appConfig.pConfig->pLogFile);
            return -1;
        }
        acceptors.push_back(pAcceptorEvent);
    }

    signal(SIGINT, signal_callback);
    signal(SIGQUIT, signal_callback);
    signal(SIGPIPE, signal_callback);

    pthread_setname_np(pthread_self(), "TMain");

    while(1) {
        if(!g_appConfig.bMainThreadExit) {
            sleep(5);
        }
        else {
            LOG_INFO("g_appConfig.nExitThreadCount[%d], g_appConfig.nRunThreadCount[%d]", g_appConfig.nExitThreadCount.load(), g_appConfig.nRunThreadCount);
            if(g_appConfig.nExitThreadCount == g_appConfig.nRunThreadCount) {
                break;
            }
            sleep(1);
        }
    }

    g_appConfig.pAppManager->flush_logrecord();

    for (AcceptorEvent* pAcceptorEvent : acceptors) {
        if(pAcceptorEvent) {
            delete pAcceptorEvent;
        }
    }

    LOG_INFO("Main Thread Exit !");
    return 0;
}

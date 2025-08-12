#include "ReactorThread.h"
#include "EpollReactor.h"
#include "log.h"

extern stAppConfig g_appConfig;

ReactorThread::ReactorThread() {
    m_pEpollReactor = NULL;
}

ReactorThread::~ReactorThread() {
    if(m_pEpollReactor != NULL) {
        stop_thread();
        delete m_pEpollReactor;
        m_pEpollReactor = NULL;
    }
}

ReactorThread* ReactorThread::instance() {
    static ReactorThread _instance;
    return &_instance;
}

bool ReactorThread::init(EpollReactor* pEpollReactor, std::string& strErrMsg)
{
    m_pEpollReactor = pEpollReactor;

    if(!IThread::init(1, strErrMsg)) {
        return false;
    }

    g_appConfig.nRunThreadCount++;
    pthread_setname_np(m_tid, "ReactorThread");
    LOG_INFO("Create ReactorThread success, threadID:%lu", m_tid);
    return true;
}

bool ReactorThread::register_tcp_event(IEvent* pIEvent, int fd, int mask)
{
    if(pIEvent == NULL) {
        LOG_ERROR("RegisterTcpEvent failed, Reason:m_pReactorHandler is NULL");
        return false;
    }

    return m_pEpollReactor->register_tcp_event(pIEvent, fd, mask);
}

bool ReactorThread::unregister_tcp_event(int fd, char* label)
{
    return m_pEpollReactor->unregister_tcp_event(fd, label);
}

void ReactorThread::start()
{
    while(1) {
        if(m_nRunStatus == enNoRunning) {
            sleep(1);
            continue;
        }

        if(m_nRunStatus == enStopping || m_nRunStatus==enStopped) {
            LOG_INFO("EpollReactorThread exit, m_nRunStatus[%s], exit !", GetThreadStatus());
            break;
        }
        
        m_pEpollReactor->handle_event(m_nRunStatus);
    }

    g_appConfig.nExitThreadCount.fetch_add(1);
}


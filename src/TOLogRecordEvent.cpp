#include "TOLogRecordEvent.h"
#include "ReactorThread.h"
#include "EpollReactor.h"
#include "AppManager.h"
#include "InnerMsg.h"
#include "log.h"
#include <sys/timerfd.h>
#include <errno.h>
#include <unistd.h>

extern stAppConfig g_appConfig;

TOLogRecordEvent::TOLogRecordEvent() {
    m_nTimeout = 1;
    m_timerfd = -1;
}

TOLogRecordEvent::~TOLogRecordEvent() {
    unregister_tcp_event();
    close_timerfd();
}

bool TOLogRecordEvent::init(int nTimeout) {
    m_timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (m_timerfd == -1) {
        LOG_ERROR("timerfd_create failed, errno: %d", errno);
        return false;
    }
    
#if 0
    struct itimerspec new_value;
    new_value.it_value.tv_sec = 0;
    new_value.it_value.tv_nsec = 500000000;
    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_nsec = 500000000;
#else
    struct itimerspec new_value;
    new_value.it_value.tv_sec = nTimeout;
    new_value.it_value.tv_nsec = 0;
    new_value.it_interval.tv_sec = nTimeout;
    new_value.it_interval.tv_nsec = 0;
#endif

    int bRet = timerfd_settime(m_timerfd, 0, &new_value, NULL);
    if (bRet < 0) {
        LOG_ERROR("timerfd_settime failed, errno: %d", errno);
        ::close(m_timerfd);
        return false;
    }

    bool bSuccess = ReactorThread::instance()->register_tcp_event(this, m_timerfd, READ_MASK);
    if(!bSuccess) {
        LOG_ERROR("register_tcp_event TOLogRecordEvent failed");
        return false;
    }

    LOG_INFO("register_tcp_event Success(TOLogRecordEvent), Listenfd is %d", m_timerfd);
    return true;
}

void TOLogRecordEvent::close_timerfd() {
    if(m_timerfd > 0) {
        ::close(m_timerfd);
        m_timerfd = -1;
    }
}

void TOLogRecordEvent::unregister_tcp_event() {
    if(m_timerfd > 0) {
        g_appConfig.pReactorThread->unregister_tcp_event(m_timerfd, (char*)"TOLogRecordEvent::~TOLogRecordEvent");
    }
}

bool TOLogRecordEvent::handle_input() {
    TimeoutLogRecordMsg* pLogRecordMsg = new TimeoutLogRecordMsg(INNER_MSG_TYPE_TIMEOUT_LOG_RECORD, m_timerfd);
    if(!pLogRecordMsg) return false;

    AppManager::instance()->add_inner_message(pLogRecordMsg);
    return true;
}

bool TOLogRecordEvent::handle_output() {
    return true;
}
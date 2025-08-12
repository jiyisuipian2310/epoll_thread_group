#ifndef __TIMEOUT_LOG_RECORD_EVENT_H__
#define __TIMEOUT_LOG_RECORD_EVENT_H__

#include "IEvent.h"
#include <iostream>
#include <string>
using namespace std;

class TOLogRecordEvent : public IEvent {
public:
    TOLogRecordEvent();
    ~TOLogRecordEvent();

    bool init(int nTimeout);
    void close_timerfd();
    void unregister_tcp_event();

    virtual bool handle_input();
    virtual bool handle_output();

private:
    int m_nTimeout;
    int m_timerfd;
};

#endif //__TIMEOUT_LOG_RECORD_EVENT_H__
#ifndef __REACTOR_H__
#define __REACTOR_H__

#include <unistd.h>
#include <atomic>
#include <iostream>
#include <string>
#include "common.h"
#include "IThread.h"
using namespace std;

class IEvent;
class EpollReactor;
class ReactorThread : public IThread {
public:
    static ReactorThread* instance();
    ReactorThread();
    virtual ~ReactorThread();

    virtual bool init(EpollReactor* pEpollReactor, std::string& strErrMsg);
    virtual void start();

    bool register_tcp_event(IEvent* pIEvent, int fd, int mask);
    bool unregister_tcp_event(int fd, char* label);

private:
    EpollReactor* m_pEpollReactor;
};
#endif

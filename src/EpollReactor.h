#ifndef __EPOLLREACTOR_H__
#define __EPOLLREACTOR_H__

#include <unordered_map>
#include <sys/epoll.h>
#include <iostream>
#include <string>
using namespace std;

#define EPOLL_EVENT_COUNT 65535

enum {
    ACCEPT_MASK  = 1<<0,
    CONNECT_MASK = 1<<1,
    READ_MASK    = 1<<2,
};

class IEvent;
class EpollReactor {
public:
    EpollReactor();
    ~EpollReactor();

    bool init(string& errmsg);
    bool handle_event(int nRunStatus);
    bool register_tcp_event(IEvent* pIEvent, int fd, int mask);
    bool unregister_tcp_event(int fd, char* lable = NULL);

private:
    int m_epfd;
    bool m_bEpollLT;
};

#endif

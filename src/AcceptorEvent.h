#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include "IEvent.h"
#include <iostream>
#include <string>
using namespace std;

class AcceptorEvent : public IEvent {
public:
    AcceptorEvent();
    ~AcceptorEvent();
    virtual bool handle_input();
    virtual bool handle_output();
    bool open_tcp(const string& strIp, int port, string& strErrMsg);

private:
    int m_tcpListenfd;
    int m_listenPort;
};

#endif

#ifndef __PROXY_SERVER_H__
#define __PROXY_SERVER_H__

#include "IEvent.h"
#include <iostream>
#include <string>
using namespace std;

class ProxyManager;
class ProxyServer : public IEvent {
public:
    ProxyServer(int socketfd, ProxyManager* proxyManager);
    ~ProxyServer();

    bool init(const string& strThreadName);
    bool unregister_tcp_event();

    void close_socket();
    int get_socket();

    virtual bool handle_input();
    virtual bool handle_output();

    int process_server_message(const string& strThreadName);

public:
    string m_strServerAddr;

private:
    int m_socketfd;
    ProxyManager* m_proxyManager;
};

#endif // __PROXY_SERVER_H__
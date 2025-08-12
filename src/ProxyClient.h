#ifndef __PROXY_CLIENT_H__
#define __PROXY_CLIENT_H__

#include "IEvent.h"
#include <iostream>
#include <string>
using namespace std;

class ProxyManager;
class ProxyClient : public IEvent {
public:
    ProxyClient(int socketfd, ProxyManager* proxyManager);
    ~ProxyClient();

    bool init(const string& strThreadName);
    bool unregister_tcp_event();
    void close_socket();
    int get_socket();

    virtual bool handle_input();
    virtual bool handle_output();

    int process_client_message(const string& strThreadName);

public:
    string m_strLocalAddr;
    string m_strClientAddr;

private:
    int m_socketfd;
    ProxyManager* m_proxyManager;
};


#endif // __PROXY_CLIENT_H__
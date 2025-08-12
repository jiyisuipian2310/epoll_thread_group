#ifndef __PROXY_MANAGER_H__
#define __PROXY_MANAGER_H__

#include "IEvent.h"
#include <iostream>
#include <string>
using namespace std;

#define RECV_BUFFER_SIZE 56*1024

enum ReturnCode {
    enSuccess = 0,
    enError = -1,
    enDisconnect = -2,
};

class ProxyClient;
class ProxyServer;

class ProxyManager {
public:
    ProxyManager(int client_sock, int server_sock);
    ~ProxyManager();

    bool init(const string& strThreadName, string strServerIp, int nServerPort);

    int get_client_socket();
    int get_server_socket();

    int process_client_message(const string& strThreadName);
    int process_server_message(const string& strThreadName);

    void release_resource();

public:
    ProxyClient* m_proxyClient;
    ProxyServer* m_proxyServer;
    
    string m_strRouteInfo;
    bool m_bRelease;

    int m_nLastTime;
};

#endif // __PROXY_MANAGER_H__

#include "ProxyManager.h"
#include "ProxyClient.h"
#include "ProxyServer.h"
#include "ReactorThread.h"
#include "EpollReactor.h"
#include "AppManager.h"
#include "InnerMsg.h"
#include <sys/timerfd.h>
#include <errno.h>
#include <unistd.h>
#include "log.h"

/************************************************************************************************* */
ProxyManager::ProxyManager(int client_sock, int server_sock): m_proxyClient(NULL), m_proxyServer(NULL)
{
    m_proxyClient = new ProxyClient(client_sock, this);
    m_proxyServer = new ProxyServer(server_sock, this);
    m_bRelease = false;
}

ProxyManager::~ProxyManager() {
    if(m_proxyClient) {
        delete m_proxyClient;
        m_proxyClient = NULL;
    }

    if(m_proxyServer) {
        delete m_proxyServer;
        m_proxyServer = NULL;
    }
}

bool ProxyManager::init(const string& strThreadName, string strServerIp, int nServerPort)
{
    bool bSuccess = m_proxyClient->init(strThreadName);
    if(!bSuccess) {
        LOG_ERROR("ThreadName: %s, ProxyClient init failed", strThreadName.data());
        return false;
    }

    bSuccess = m_proxyServer->init(strThreadName);
    if(!bSuccess) {
        m_proxyClient->unregister_tcp_event();
        LOG_ERROR("ThreadName: %s, ProxyServer init failed", strThreadName.data());
        return false;
    }

    m_nLastTime = time(0);

    m_strRouteInfo.append(m_proxyClient->m_strClientAddr).append(" -> ");
    m_strRouteInfo.append(m_proxyClient->m_strLocalAddr).append(" -> ");

    m_proxyServer->m_strServerAddr.append(strServerIp).append(":").append(std::to_string(nServerPort));
    m_strRouteInfo.append(m_proxyServer->m_strServerAddr);
    LOG_INFO("This is new proxy connection, ThreadName: %s, routeInfo: %s", strThreadName.data(), m_strRouteInfo.c_str());
    return true;
}

int ProxyManager::get_client_socket() {
    return m_proxyClient->get_socket();
}

int ProxyManager::get_server_socket() {
    return m_proxyServer->get_socket();
}

int ProxyManager::process_client_message(const string& strThreadName) {
    return m_proxyClient->process_client_message(strThreadName);
}

int ProxyManager::process_server_message(const string& strThreadName) {
    return m_proxyServer->process_server_message(strThreadName);
}

void ProxyManager::release_resource()
{
    m_proxyClient->unregister_tcp_event();
    m_proxyClient->close_socket();

    m_proxyServer->unregister_tcp_event();
    m_proxyServer->close_socket();
}

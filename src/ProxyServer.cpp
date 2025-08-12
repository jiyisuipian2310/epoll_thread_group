#include "ProxyServer.h"
#include "ProxyManager.h"
#include "ReactorThread.h"
#include "EpollReactor.h"
#include "AppManager.h"
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <strings.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "log.h"

ProxyServer::ProxyServer(int socketfd, ProxyManager* proxyManager) {
    m_socketfd = socketfd;
    m_proxyManager = proxyManager;
}

ProxyServer::~ProxyServer() {
    close_socket();
}

bool ProxyServer::init(const string& strThreadName)
{
    int flags = fcntl(m_socketfd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(m_socketfd, F_SETFL, flags);

    struct linger so_linger;
    so_linger.l_onoff = 1;
    so_linger.l_linger = 0;
    setsockopt(m_socketfd, SOL_SOCKET, SO_LINGER, (const void*)&so_linger , sizeof(so_linger));

    int recvBufferSize = 64*1024;
    setsockopt(m_socketfd, SOL_SOCKET, SO_RCVBUF, (const void*)&recvBufferSize, sizeof(recvBufferSize));

    bool bSuccess = ReactorThread::instance()->register_tcp_event(this, m_socketfd, READ_MASK);
    if(!bSuccess) {
        LOG_ERROR("ThreadName: %s, register_tcp_event ProxyServer failed", strThreadName.data());
        return false;
    }

    return true;
}

void ProxyServer::close_socket()
{
    if(m_socketfd > 0) {
        ::close(m_socketfd);
        m_socketfd = -1;
    }
}

int ProxyServer::get_socket()
{
    return m_socketfd;
}

bool ProxyServer::unregister_tcp_event()
{
    if(m_socketfd > 0) {
        ReactorThread::instance()->unregister_tcp_event(m_socketfd, (char*)"ProxyServer socketfd");
    }

    return true;
}

bool ProxyServer::handle_output() {
    return true;
}

bool ProxyServer::handle_input()
{
    if(m_proxyManager->m_bRelease == true) {
        return true;
    }

    m_proxyManager->m_nLastTime = time(0);

    ServerMsg* pServerMsg = new ServerMsg(INNER_MSG_TYPE_SERVER_DATA_RECEIVE, m_socketfd);
    if(!pServerMsg) return false;

    pServerMsg->SetProxyManager(m_proxyManager);
    AppManager::instance()->add_inner_message(pServerMsg);
    return true;
}

int ProxyServer::process_server_message(const string& strThreadName)
{
    int nTotalLength = 0;
    int recvFlag = 0;

    char arrDataBuffer[RECV_BUFFER_SIZE] = { 0 };

    while(1) {
        int nReadLength = ::recv(m_socketfd, arrDataBuffer+nTotalLength, RECV_BUFFER_SIZE - nTotalLength, 0);
        int lastError = errno;
        if(nReadLength < 0) {
            if(lastError == EWOULDBLOCK || lastError == EAGAIN || lastError == EINTR) {
                break;
            }

            recvFlag = 1;
            break; // peer close
        } else if(nReadLength == 0) {
            recvFlag = 1;
            break; //peer close
        } else {
            nTotalLength += nReadLength;
            if(nTotalLength > RECV_BUFFER_SIZE) {
                LOG_ERROR("nTotalLength(%d) more than limitLength(%d), close connection !", nTotalLength, RECV_BUFFER_SIZE);
                return ReturnCode::enError;
            }
            continue;
        }
    }

    if(recvFlag == 1) {
        //peer close        
        m_proxyManager->m_bRelease == true;
        LOG_INFO("ThreadName: %s, ProxyServer[%s] Close Connect, routeInfo: %s", strThreadName.data(), m_strServerAddr.c_str(), m_proxyManager->m_strRouteInfo.c_str());
        unregister_tcp_event();
        close_socket();
        return ReturnCode::enDisconnect;
    }

    int fd = m_proxyManager->get_client_socket();
    if(fd > 0) {
        ::send(fd, arrDataBuffer, nTotalLength, 0);
    }

    return ReturnCode::enSuccess;
}
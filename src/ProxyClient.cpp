#include "ProxyClient.h"
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

ProxyClient::ProxyClient(int socketfd, ProxyManager* proxyManager) {
    m_socketfd = socketfd;
    m_proxyManager = proxyManager;
}

ProxyClient::~ProxyClient() {
    close_socket();
}

bool ProxyClient::init(const string& strThreadName)
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
        LOG_ERROR("InnerMsgThreadName: %s, register_tcp_event ProxyClient failed", strThreadName.data());
        return false;
    }

    struct sockaddr_in sockAddr;
    socklen_t nSockAddrLen = sizeof(sockAddr);
    getpeername(m_socketfd, (struct sockaddr*)&sockAddr, &nSockAddrLen);

    char arrIpAddr[56] = { 0 };
    m_strClientAddr = inet_ntop(AF_INET, &sockAddr.sin_addr, arrIpAddr, sizeof(arrIpAddr));
    m_strClientAddr.append(":").append(std::to_string(ntohs(sockAddr.sin_port)));

    memset(&arrIpAddr, 0, sizeof(arrIpAddr));
    nSockAddrLen = sizeof(sockAddr);
    getsockname(m_socketfd, (struct sockaddr*)&sockAddr, &nSockAddrLen);
    m_strLocalAddr = inet_ntop(AF_INET, &sockAddr.sin_addr, arrIpAddr, sizeof(arrIpAddr));
    m_strLocalAddr.append(":").append(std::to_string(ntohs(sockAddr.sin_port)));
    return true;
}

bool ProxyClient::unregister_tcp_event() {
    if(m_socketfd > 0) {
        ReactorThread::instance()->unregister_tcp_event(m_socketfd, (char*)"ProxyClient socketfd");
    }
    return true;
}

void ProxyClient::close_socket() {
    if(m_socketfd > 0) {
        ::close(m_socketfd);
        m_socketfd = -1;
    }
}

int ProxyClient::get_socket() {
    return m_socketfd;
}

bool ProxyClient::handle_output() {
    return true;
}

bool ProxyClient::handle_input()
{
    if(m_proxyManager->m_bRelease == true) {
        return true;
    }

    m_proxyManager->m_nLastTime = time(0);

    ClientMsg* pClientMsg = new ClientMsg(INNER_MSG_TYPE_CLIENT_DATA_RECEIVE, m_socketfd);
    if(!pClientMsg) return false;

    pClientMsg->SetProxyManager(m_proxyManager);
    AppManager::instance()->add_inner_message(pClientMsg);
    return true;
}

int ProxyClient::process_client_message(const string& strThreadName)
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
        LOG_INFO("InnerThreadName: %s, ProxyClient[%s] Close Connect, routeInfo: %s", strThreadName.data(), m_strClientAddr.c_str(), m_proxyManager->m_strRouteInfo.c_str());
        unregister_tcp_event();
        close_socket();
        return ReturnCode::enDisconnect;
    }

    int fd = m_proxyManager->get_server_socket();
    if(fd > 0) {
        ::send(fd, arrDataBuffer, nTotalLength, 0);
    }

    return ReturnCode::enSuccess;
}
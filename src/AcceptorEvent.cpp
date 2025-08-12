#include "AcceptorEvent.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <strings.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ReactorThread.h"
#include "EpollReactor.h"
#include "log.h"
#include "AppManager.h"

AcceptorEvent::AcceptorEvent() {
    m_tcpListenfd = -1;
    m_listenPort = -1;
}

AcceptorEvent::~AcceptorEvent() {
}

bool AcceptorEvent::handle_output() {
    return true;
}

bool AcceptorEvent::handle_input()
{
    struct sockaddr_in clientAddr;
    socklen_t nAddrLen = sizeof(clientAddr);
    while(1) {
        int socketfd = accept(m_tcpListenfd, (struct sockaddr*)&clientAddr, &nAddrLen);
        if(socketfd < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                return true;
            }
            continue;
        }

        AcceptMsg* pAcceptMsg = new AcceptMsg(INNER_MSG_TYPE_ACCEPT, socketfd);
        if(!pAcceptMsg) return false;

        pAcceptMsg->SetListenPort(m_listenPort);
        AppManager::instance()->add_inner_message(pAcceptMsg);
    }

    return true;
}

/************************
struct sockaddr_in {
    short int sin_family; 
    unsigned short int sin_port; 
    struct in_addr sin_addr;
    unsigned char sin_zero[8];
};
************************/

bool AcceptorEvent::open_tcp(const string& strIp, int port, string& strErrMsg)
{
    char errbuf[1024] = { 0 };
    m_listenPort = port;
    m_tcpListenfd = socket(AF_INET, SOCK_STREAM, 0);

    int iflag = fcntl(m_tcpListenfd, F_GETFL);
    iflag |= O_NONBLOCK;
    fcntl(m_tcpListenfd, F_SETFL, iflag);

    struct sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    inet_aton(strIp.c_str(), &serverAddr.sin_addr);
    serverAddr.sin_port = htons(port);

    int op = 1;
    setsockopt(m_tcpListenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&op , sizeof(int));

    if(bind(m_tcpListenfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        sprintf(errbuf, "bind(%s:%d) exec failed, reason:%s", strIp.c_str(), port, strerror(errno));
        strErrMsg = errbuf;
        return false;
    }

    if(listen(m_tcpListenfd, 65535) < 0) {
        sprintf(errbuf, "listen(%s:%d) exec failed, reason:%s", strIp.c_str(), port, strerror(errno));
        strErrMsg = errbuf;
        return false;
    }

    bool bRet = ReactorThread::instance()->register_tcp_event(this, m_tcpListenfd, ACCEPT_MASK);
    if(bRet) {
        LOG_INFO("register_tcp_event Success(AcceptorEvent), ListenAddr:%s:%d(fd:%d)", strIp.c_str(), port, m_tcpListenfd);
        return true;
    }

    sprintf(errbuf, "register_tcp_event Failed(AcceptorEvent), ListenAddr:%s:%d(fd:%d)", strIp.c_str(), port, m_tcpListenfd);
    strErrMsg = errbuf;
    return false;
}
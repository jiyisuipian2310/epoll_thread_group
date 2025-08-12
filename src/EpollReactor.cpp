#include <unistd.h>
#include <sys/epoll.h>
#include <string.h>
#include "EpollReactor.h"
#include "IEvent.h"
#include "ReactorThread.h"
#include "log.h"

EpollReactor::EpollReactor() {
    m_bEpollLT = false;
    m_epfd = -1;
}

EpollReactor::~EpollReactor() {
    if(m_epfd > 0) {
        ::close(m_epfd);
    }
}

bool EpollReactor::init(string& errmsg) {
    m_epfd = epoll_create(EPOLL_EVENT_COUNT);
    if(m_epfd < 0) {
        errmsg.append("epoll_create failed, errno:").append(strerror(errno));
        return false;
    }

    LOG_INFO("epoll_create Success, epoll fd: %d", m_epfd);
    return true;
}

bool EpollReactor::handle_event(int nRunStatus)
{
    struct epoll_event arrEpollEvent[EPOLL_EVENT_COUNT];
    int iReadyfd = ::epoll_wait(m_epfd, arrEpollEvent, EPOLL_EVENT_COUNT, 100); //100ms
    for(int i = 0; i < iReadyfd; i++) {
        IEvent* pIEvent = (IEvent*)arrEpollEvent[i].data.ptr;
        if(pIEvent == NULL) {
            continue;
        }

        if(nRunStatus == enStopping || nRunStatus == enStopped) {
            break;
        }
        
        if(arrEpollEvent[i].events & EPOLLIN) {
            pIEvent->handle_input();
        } else if(arrEpollEvent[i].events & EPOLLOUT) {
            pIEvent->handle_output();
        }
    }
    return true;
}

/*
struct epoll_event {
    uint32_t     events;    // 需要监控的事件类型（位掩码）
    epoll_data_t data;      // 用户数据（可以存储文件描述符、指针等）
};

typedef union epoll_data {
    void        *ptr;   // 可以存储任意指针
    int          fd;    // 可以存储文件描述符
    uint32_t     u32;   // 32 位无符号整数
    uint64_t     u64;   // 64 位无符号整数
} epoll_data_t;
*/

bool EpollReactor::register_tcp_event(IEvent* pIEvent, int fd, int mask)
{
    if(pIEvent == NULL) {
        LOG_ERROR("register_tcp_event failed, pIEvent is null");
        return false;
    }

    if(fd < 0) {
        LOG_ERROR("register_tcp_event failed, fd is invalid");
        return false;
    }

    int op = 0;
    struct epoll_event ev;

    op = EPOLL_CTL_ADD;
    ev.data.ptr = (void*)pIEvent;
   
    if(mask == ACCEPT_MASK || mask == READ_MASK) {
        ev.events = EPOLLIN;
    } else if(mask == CONNECT_MASK) {
        ev.events = EPOLLOUT;
    }

    ev.events = m_bEpollLT ? ev.events:ev.events|EPOLLET;
    if(epoll_ctl(m_epfd, op, fd, &ev) < 0) {
        LOG_ERROR("register_tcp_event failed, epoll_ctl failed, fd:%d, errno:%d", fd, errno);
        return false;
    }

    return true;
}

bool EpollReactor::unregister_tcp_event(int fd, char* label)
{
    if(label != NULL)
        LOG_DEBUG("Begin unregister_tcp_event, label: %s, fd:%d", label, fd);
    else
        LOG_DEBUG("Begin unregister_tcp_event, fd:%d", fd);

    epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, NULL);
    return true;
}

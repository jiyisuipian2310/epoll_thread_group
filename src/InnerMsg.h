#ifndef __INNERMSG_H__
#define __INNERMSG_H__

#include <iostream>
#include <string>
using namespace std;

enum {
    INNER_MSG_TYPE_ACCEPT               = 0,
    INNER_MSG_TYPE_CLIENT_DATA_RECEIVE  = 1,
    INNER_MSG_TYPE_SERVER_DATA_RECEIVE  = 2,
    INNER_MSG_TYPE_TIMEOUT_DISCONNECT   = 3,
    INNER_MSG_TYPE_TIMEOUT_LOG_RECORD   = 4,
    INNER_MSG_TYPE_RELEASE_RESOURCE     = 99,
};

class ProxyManager;
class IInnerMsg {
public:
    IInnerMsg(int msgType, int socketfd): m_msgType(msgType), m_socketfd(socketfd) {}
    virtual ~IInnerMsg() {}
    virtual void ProcessInnerMsg(IInnerMsg* pInnerMsg, const string& strThreadName) = 0;
public:
    int m_msgType;
    int m_socketfd;
};

class AcceptMsg : public IInnerMsg {
public:
    AcceptMsg(int msgType, int socketfd) :IInnerMsg(msgType, socketfd) {}
    ~AcceptMsg() {}
    virtual void ProcessInnerMsg(IInnerMsg* pInnerMsg, const string& strThreadName);
    void SetListenPort(int listenPort) { m_listenPort = listenPort; }

public:
    int m_listenPort;
};

class TimeoutDisconnectMsg : public IInnerMsg {
public:
    TimeoutDisconnectMsg(int msgType, int socketfd) :IInnerMsg(msgType, socketfd) {}
    ~TimeoutDisconnectMsg() {}
    virtual void ProcessInnerMsg(IInnerMsg* pInnerMsg, const string& strThreadName);
};

class TimeoutLogRecordMsg : public IInnerMsg {
public:
    TimeoutLogRecordMsg(int msgType, int socketfd) :IInnerMsg(msgType, socketfd) {}
    ~TimeoutLogRecordMsg() {}
    virtual void ProcessInnerMsg(IInnerMsg* pInnerMsg, const string& strThreadName);
};

class ReleaseResourceMsg : public IInnerMsg {
public:
    ReleaseResourceMsg(int msgType, int socketfd) :IInnerMsg(msgType, socketfd) {}
    ~ReleaseResourceMsg() {}
    virtual void ProcessInnerMsg(IInnerMsg* pInnerMsg, const string& strThreadName);
    
    void SetProxyManager(ProxyManager* pProxyManager) { m_pProxyManager = pProxyManager; }
    ProxyManager* GetProxyManager() { return m_pProxyManager; }
private:
    ProxyManager* m_pProxyManager;  
};

class ClientMsg : public IInnerMsg {
public:
    ClientMsg(int msgType, int socketfd) :IInnerMsg(msgType, socketfd) {}
    ~ClientMsg() {}
    virtual void ProcessInnerMsg(IInnerMsg* pInnerMsg, const string& strThreadName);

    void SetProxyManager(ProxyManager* pProxyManager) { m_pProxyManager = pProxyManager; }
    ProxyManager* GetProxyManager() { return m_pProxyManager; }
private:
    ProxyManager* m_pProxyManager;
};

class ServerMsg : public IInnerMsg {
public:
    ServerMsg(int msgType, int socketfd) :IInnerMsg(msgType, socketfd) {}
    ~ServerMsg() {}
    virtual void ProcessInnerMsg(IInnerMsg* pInnerMsg, const string& strThreadName);

    void SetProxyManager(ProxyManager* pProxyManager) { m_pProxyManager = pProxyManager; }
    ProxyManager* GetProxyManager() { return m_pProxyManager; }
private:
    ProxyManager* m_pProxyManager;
};
#endif

#ifndef __APPMANAGER_H__
#define __APPMANAGER_H__

#include "common.h"
#include "InnerMsg.h"
#include <pthread.h>

class AcceptMsgThread;
class TODisconnectEvent;
class TOLogRecordEvent;
class ProxyManager;
class ClientServerMsgThread;
class TimeoutDisconnectMsgThread;
class TimeoutLogRecordMsgThread;
class AppManager {
public:
    static AppManager* instance();

    bool init(string& strErrMsg);

    void stop_thread();

    bool add_inner_message(IInnerMsg* pInnerMsg);

    int process_release_resource_message(ReleaseResourceMsg* pReleaseResourceMsg, const string& strThreadName);

    int process_accept_message(AcceptMsg* pAcceptMsg, const string& strThreadName);

    int process_client_message(ClientMsg* pClientMsg, const string& strThreadName);

    int process_server_message(ServerMsg* pServerMsg, const string& strThreadName);

    int process_timeout_disconnect_message(TimeoutDisconnectMsg* pTimeoutMsg, const string& strThreadName);

    int process_timeout_logrecord_message(TimeoutLogRecordMsg* pLogRecordMsg, const string& strThreadName);

    void flush_logrecord();

    void add_proxy_manager(ProxyManager* pProxyManager);
    void delete_proxy_manager(ProxyManager* pProxyManager);

private:
    AppManager();
    ~AppManager();

private:
    AcceptMsgThread* m_arrAcceptMsgThread[256];
    ClientServerMsgThread* m_arrClientServerMsgThread[256];
    TimeoutDisconnectMsgThread* m_arrTimeoutDisconnectMsgThread[1];
    TimeoutLogRecordMsgThread* m_arrTimeoutLogRecordMsgThread[1];

    TODisconnectEvent* m_pTimeoutDisconnectEvent;
    TOLogRecordEvent* m_pTimeoutLogRecordEvent;

    pthread_mutex_t m_mutex_proxyManager;
    std::set<ProxyManager*> m_setProxyManager;
};

#endif
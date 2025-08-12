#include "AppManager.h"
#include "InnerMsgThread.h"
#include "InnerMsg.h"
#include "ReactorThread.h"
#include "log.h"
#include "TODisconnectEvent.h"
#include "TOLogRecordEvent.h"
#include "ProxyManager.h"
#include "ProxyClient.h"
#include "ProxyServer.h"
#include "ConfigInit.h"
#include "AutoLock.h"

extern stAppConfig g_appConfig;

AppManager* AppManager::instance() {
    static AppManager _instance;
    return &_instance;
}

AppManager::AppManager() {
}

AppManager::~AppManager() {
    if(m_pTimeoutLogRecordEvent) {
        delete m_pTimeoutLogRecordEvent;
        m_pTimeoutLogRecordEvent = NULL;
    }

    if(m_pTimeoutDisconnectEvent) {
        delete m_pTimeoutDisconnectEvent;
        m_pTimeoutDisconnectEvent = NULL;
    }
}

bool AppManager::init(string& strErrMsg)
{
    int nThreadNo = 0;
    char errmsg[1024] = {0};
    
    int nThreadCount = g_appConfig.pConfig->m_nThreadCount;
    nThreadCount = nThreadCount>256 ? 256 : nThreadCount;

    for(int i = 0; i < nThreadCount; i++) {
        AcceptMsgThread* pInnerMsgThread = new AcceptMsgThread();
        if(pInnerMsgThread == NULL) {
            sprintf(errmsg, "Error: Create AcceptMsgThread Failed, nThreadNo: %d", nThreadNo);
            goto failed;
        }

        if(!(pInnerMsgThread->init(nThreadNo, (char*)"TInnerAccept", strErrMsg))) {
            goto failed;
        }

        m_arrAcceptMsgThread[i] = pInnerMsgThread;
        pInnerMsgThread->start_thread();
        nThreadNo++;
    }
    LOG_INFO("Create AcceptMsgThread Success, InnerMsgThread Number: %d", nThreadCount);

    /****************************************************************************** */
    nThreadNo = 0;
    for(int i = 0; i < nThreadCount; i++) {
        ClientServerMsgThread* pInnerMsgThread = new ClientServerMsgThread();
        if(pInnerMsgThread == NULL) {
            sprintf(errmsg, "Error: Create ClientServerMsgThread Failed, nThreadNo: %d", nThreadNo);
            goto failed;
        }

        if(!(pInnerMsgThread->init(nThreadNo, (char*)"TInnerCS", strErrMsg))) {
            goto failed;
        }

        m_arrClientServerMsgThread[i] = pInnerMsgThread;
        pInnerMsgThread->start_thread();
        nThreadNo++;
    }
    LOG_INFO("Create ClientServerMsgThread Success, InnerMsgThread Number: %d", nThreadCount);

    /****************************************************************************** */
    nThreadNo = 0;
    for(int i = 0; i < 1; i++) {
        TimeoutDisconnectMsgThread* pInnerMsgThread = new TimeoutDisconnectMsgThread();
        if(pInnerMsgThread == NULL) {
            sprintf(errmsg, "Error: Create TimeoutDisconnectMsgThread Failed, nThreadNo: %d", nThreadNo);
            goto failed;
        }

        if(!(pInnerMsgThread->init(nThreadNo, (char*)"TInnerTOD", strErrMsg))) {
            goto failed;
        }

        m_arrTimeoutDisconnectMsgThread[i] = pInnerMsgThread;
        pInnerMsgThread->start_thread();
        nThreadNo++;
    }
    LOG_INFO("Create TimeoutDisconnectMsgThread Success, InnerMsgThread Number: %d", 1);

    /****************************************************************************** */
    nThreadNo = 0;
    for(int i = 0; i < 1; i++) {
        TimeoutLogRecordMsgThread* pInnerMsgThread = new TimeoutLogRecordMsgThread();
        if(pInnerMsgThread == NULL) {
            sprintf(errmsg, "Error: Create TimeoutLogRecordMsgThread Failed, nThreadNo: %d", nThreadNo);
            goto failed;
        }

        if(!(pInnerMsgThread->init(nThreadNo, (char*)"TInnerTOLR", strErrMsg))) {
            goto failed;
        }

        m_arrTimeoutLogRecordMsgThread[i] = pInnerMsgThread;
        pInnerMsgThread->start_thread();
        nThreadNo++;
    }
    LOG_INFO("Create TimeoutLogRecordMsgThread Success, InnerMsgThread Number: %d", 1);

    m_pTimeoutDisconnectEvent = new TODisconnectEvent();
    if(m_pTimeoutDisconnectEvent == NULL) {
        sprintf(errmsg, "Error: Create TODisconnectEvent Failed");
        goto failed;
    }

    if(!m_pTimeoutDisconnectEvent->init(5)) {
        sprintf(errmsg, "Error: TimeoutDisconnectEvent init Failed");
        goto failed;
    }

    m_pTimeoutLogRecordEvent = new TOLogRecordEvent();
    if(m_pTimeoutLogRecordEvent == NULL) {
        sprintf(errmsg, "Error: Create TOLogRecordEvent Failed");
        goto failed;
    }

    if(!m_pTimeoutLogRecordEvent->init(1)) {
        sprintf(errmsg, "Error: TimeoutLogRecordEvent init Failed");
        goto failed;
    }

    pthread_mutex_init(&m_mutex_proxyManager, NULL);
    return true;

failed:
    if(errmsg[0] != '\0') strErrMsg = errmsg;
    return false;
}

void AppManager::stop_thread() {
    int nThreadCount = g_appConfig.pConfig->m_nThreadCount;
    nThreadCount = nThreadCount>256?256:nThreadCount;

    for(int i = 0; i < 1; i++) {
        if(m_arrTimeoutDisconnectMsgThread[i]) {
            m_arrTimeoutDisconnectMsgThread[i]->stop_thread();
        }
    }

    for(int i = 0; i < 1; i++) {
        if(m_arrTimeoutLogRecordMsgThread[i]) {
            m_arrTimeoutLogRecordMsgThread[i]->stop_thread();
        }
    }

    for(int i = 0; i < nThreadCount; i++) {
        if(m_arrAcceptMsgThread[i]) {
            m_arrAcceptMsgThread[i]->stop_thread();
        }
    }

    for(int i = 0; i < nThreadCount; i++) {
        if(m_arrClientServerMsgThread[i]) {
            m_arrClientServerMsgThread[i]->stop_thread();
        }
    }
}

bool AppManager::add_inner_message(IInnerMsg* pInnerMsg)
{
    if(pInnerMsg == NULL) { return false; }

    if(pInnerMsg->m_socketfd < 0) {
        LOG_INFO("Connect have be released");
        return false;
    }

    AcceptMsgThread* pMsgThread = NULL;
    int index = pInnerMsg->m_socketfd % (g_appConfig.pConfig->m_nThreadCount);
    switch(pInnerMsg->m_msgType) {
        case INNER_MSG_TYPE_TIMEOUT_DISCONNECT:
            pMsgThread = m_arrTimeoutDisconnectMsgThread[0];
            break;
        case INNER_MSG_TYPE_TIMEOUT_LOG_RECORD:
            pMsgThread = m_arrTimeoutLogRecordMsgThread[0];
            break;
        case INNER_MSG_TYPE_ACCEPT:
            pMsgThread = m_arrAcceptMsgThread[index];
            break;
        case INNER_MSG_TYPE_CLIENT_DATA_RECEIVE: 
        case INNER_MSG_TYPE_SERVER_DATA_RECEIVE:
            pMsgThread = m_arrClientServerMsgThread[index];
            break;
        default:
            return false;
    }

    if(pMsgThread == NULL) {
        LOG_ERROR("InnerMsgThread is NULL, pInnerMsg->m_msgType:%d", pInnerMsg->m_msgType);
        return false;
    }

    pMsgThread->add_inner_message(pInnerMsg);
    return true;
}

void AppManager::add_proxy_manager(ProxyManager* pProxyManager)
{
    if(pProxyManager == NULL) { return; }

    AutoMutexLock lock(&m_mutex_proxyManager);
    if(m_setProxyManager.find(pProxyManager) == m_setProxyManager.end()) {
        m_setProxyManager.insert(pProxyManager);
    }
}

void AppManager::delete_proxy_manager(ProxyManager* pProxyManager)
{
    if(pProxyManager == NULL) { return; }

    AutoMutexLock lock(&m_mutex_proxyManager);
    if(m_setProxyManager.find(pProxyManager) != m_setProxyManager.end()) {
        m_setProxyManager.erase(pProxyManager);
    }
}

int AppManager::process_accept_message(AcceptMsg* pAcceptMsg, const string& strThreadName)
{
    if(pAcceptMsg == NULL) { return -1; }

    string strUpstreamIp = "192.168.104.100";
    int nUpstreamPort = 10000;

    int upstreamsock = socket(AF_INET, SOCK_STREAM, 0);
    if (upstreamsock == -1) {
        ::close(pAcceptMsg->m_socketfd);
        LOG_ERROR("ThreadName: %s, Create socket failed, resaon:%s", strThreadName.data(), strerror(errno));
        return -1;
    }

    struct sockaddr_in upstreamaddr;
    memset(&upstreamaddr, 0, sizeof(upstreamaddr));
    upstreamaddr.sin_family = AF_INET;
    upstreamaddr.sin_addr.s_addr = inet_addr(strUpstreamIp.data());
    upstreamaddr.sin_port = htons(nUpstreamPort);

    // 连接服务器
    if (connect(upstreamsock, (struct sockaddr *)&upstreamaddr, sizeof(upstreamaddr)) == -1) {
        ::close(pAcceptMsg->m_socketfd);
        LOG_ERROR("InnerMsgThreadName: %s, connect upstreamaddr(%s:%d) failed, resaon:%s", strThreadName.data(), strUpstreamIp.data(), nUpstreamPort, strerror(errno));
        return -1;
    }

    ProxyManager* pProxyManager = new ProxyManager(pAcceptMsg->m_socketfd, upstreamsock);
    if(pProxyManager == NULL) {
        LOG_ERROR("InnerMsgThreadName: %s, Create ProxyManager failed", strThreadName.data());
        ::close(pAcceptMsg->m_socketfd);
        ::close(upstreamsock);
        return -1;
    }

    if(!pProxyManager->init(strThreadName, strUpstreamIp, nUpstreamPort)) {
        delete pProxyManager;
        return -1;
    }

    add_proxy_manager(pProxyManager);
    return 0;
}

int AppManager::process_release_resource_message(ReleaseResourceMsg* pReleaseResourceMsg, const string& strThreadName)
{
    if(pReleaseResourceMsg == NULL) { return -1; }

    ProxyManager* pProxyManager = pReleaseResourceMsg->GetProxyManager();
    if(pProxyManager == NULL) { return -1;}

    if(pProxyManager->m_bRelease) {
        return 0;
    }

    pProxyManager->release_resource();
    delete pProxyManager;

    return 0;
}

int AppManager::process_client_message(ClientMsg* pClientMsg, const string& strThreadName)
{
    if(pClientMsg == NULL) { return -1; }

    ProxyManager* proxyManager = pClientMsg->GetProxyManager();
    if(proxyManager == NULL) { return -1;}
    
    if(proxyManager->m_bRelease) {
        return 0;
    }

    int nRet = proxyManager->process_client_message(strThreadName);
    if(nRet == ReturnCode::enDisconnect) {
        delete_proxy_manager(proxyManager);
        proxyManager->release_resource();
        delete proxyManager;
    }

    return nRet;
}

int AppManager::process_server_message(ServerMsg* pServerMsg, const string& strThreadName)
{
    if(pServerMsg == NULL) { return -1; }

    ProxyManager* proxyManager = pServerMsg->GetProxyManager();
    if(proxyManager == NULL) { return -1;}

    if(proxyManager->m_bRelease) {
        return 0;
    }

    int nRet = proxyManager->process_server_message(strThreadName);
    if(nRet == ReturnCode::enDisconnect) {
        delete_proxy_manager(proxyManager);
        proxyManager->release_resource();
        delete proxyManager;
    }
}

void AppManager::flush_logrecord()
{
    string strLog;
    if(g_appConfig.pConfig->m_strLogOutput == "console") {
        while(g_logger.pop(strLog)) {
            printf("%s\n", strLog.data());
        }
    }
    else {
        while(g_logger.pop(strLog)) {
            strLog.append("\n");
            fwrite(strLog.data(), strLog.size(), 1, g_appConfig.pConfig->pLogFile);
        }

        fflush(g_appConfig.pConfig->pLogFile);
    }
}

int AppManager::process_timeout_disconnect_message(TimeoutDisconnectMsg* pTimeoutDisconnectMsg, const string& strThreadName)
{
    uint64_t exp;
    int m_nCurrentTime = time(0);
    ProxyManager* pProxyManager = NULL;

    AutoMutexLock lock(&m_mutex_proxyManager);
    for (auto it = m_setProxyManager.begin(); it != m_setProxyManager.end();)
    {
        pProxyManager = *it;
        int nTimespan = m_nCurrentTime - pProxyManager->m_nLastTime;
        if (nTimespan >= g_appConfig.pConfig->m_nTimeoutDisconnect) {
            it = m_setProxyManager.erase(it);  // erase 返回下一个有效迭代器
            LOG_WARN("timeout(%d seconds) disconnect, routeInfo: %s", g_appConfig.pConfig->m_nTimeoutDisconnect, pProxyManager->m_strRouteInfo.data());
            pProxyManager->release_resource();
            delete pProxyManager;
            pProxyManager = NULL;
        } else {
            ++it;  // 只有不删除时才递增
        }
    }

    read(pTimeoutDisconnectMsg->m_socketfd, &exp, sizeof(uint64_t));
    return 0;
}

int AppManager::process_timeout_logrecord_message(TimeoutLogRecordMsg* pLogRecordMsg, const string& strThreadName)
{
    int item_count = 0;
    string strLog;

    if(g_appConfig.pConfig->m_strLogOutput == "console") {
        while(g_logger.pop(strLog) && item_count < 5000) {
            item_count++;
            printf("%s\n", strLog.data());
        }
    }
    else {
        string strCurrentDate = get_current_time("%Y%m%d");
        if(strCurrentDate != g_appConfig.strLastDate) {
            g_appConfig.strLastDate = strCurrentDate;
            string strLogName = g_appConfig.pConfig->m_strLogPath + "/server_" + strCurrentDate + ".log";
            FILE* pLogFile = fopen(strLogName.data(), "a+");
            if(pLogFile == NULL) {
                LOG_ERROR("InnerMsgThreadName: %s, Open log file(%s) failed, resaon:%s", strThreadName.data(), strLogName.data(), strerror(errno));
                return -1;
            }
            else {
                fclose(g_appConfig.pConfig->pLogFile);
                g_appConfig.pConfig->pLogFile = pLogFile;
            }
        }

        while(g_logger.pop(strLog) && item_count < 5000) {
            item_count++;
            strLog.append("\n");
            fwrite(strLog.data(), strLog.size(), 1, g_appConfig.pConfig->pLogFile);
        }

        if(item_count > 0) fflush(g_appConfig.pConfig->pLogFile);
    }
    
    uint64_t exp;
    read(pLogRecordMsg->m_socketfd, &exp, sizeof(uint64_t));
    return 0;
}
#include <unistd.h>
#include <arpa/inet.h>
#include <atomic>
#include "InnerMsgThread.h"
#include "InnerMsg.h"
#include "common.h"
#include "log.h"
#include <pthread.h>

extern stAppConfig g_appConfig;

bool AcceptMsgThread::init(int nThreadNo, string strThreadName, string& strErrMsg)
{
    if(!IThread::init(nThreadNo, strErrMsg)) {
        return false;
    }

    m_strThreadName = strThreadName;
    m_strThreadName.append(std::to_string(nThreadNo));
    pthread_setname_np(m_tid, m_strThreadName.data());
    g_appConfig.nRunThreadCount++;
    return true;
}

void AcceptMsgThread::start() {
    while(1) {
        if(m_nRunStatus == enNoRunning) {
            sleep(1);
            continue;
        }

        if(m_nRunStatus == enStopping || m_nRunStatus==enStopped) {
            LOG_INFO("InnerMsgThread[%s] exit, m_nRunStatus[%s], exit !", m_strThreadName.data(), GetThreadStatus());
            break;
        }

        IInnerMsg* pInnerMsg = GetInnerMsg();
        if(pInnerMsg != NULL) {
            pInnerMsg->ProcessInnerMsg(pInnerMsg, m_strThreadName);
            delete pInnerMsg;
            pInnerMsg = NULL;
        }
    }

    g_appConfig.nExitThreadCount.fetch_add(1);
}

void AcceptMsgThread::add_inner_message(IInnerMsg* pInnerMsg)
{
    if(pInnerMsg == NULL) return;

    while (!m_msgqueue.push(pInnerMsg)) {
        //m_msgqueue is full, waiting ...
        if(m_nRunStatus == enStopping || m_nRunStatus==enStopped) {
            break;
        }
        usleep(1000);
    }
}

IInnerMsg* AcceptMsgThread::GetInnerMsg()
{
    IInnerMsg* pInnerMsg = NULL;
    while (!m_msgqueue.pop(pInnerMsg)) {
        //m_msgqueue is empty, waiting ...
        if(m_nRunStatus == enStopping || m_nRunStatus==enStopped) {
            break;
        }
        usleep(1000);
    }
    return pInnerMsg;
}
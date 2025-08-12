#include <errno.h>
#include "IThread.h"
#include "EpollReactor.h"
#include "log.h"
#include <iostream>
#include <string>

void* IThread::ThreadFunc(void* arg) {
    IThread* pThread = (IThread*)arg;
    pThread->start();
    return NULL;
}

IThread::IThread() {
    m_tid = 0;
    m_nRunStatus = enNoRunning;
    m_nThreadNo = -1;
}

IThread::~IThread() {
}

bool IThread::init(int nThreadNo, string& strErrMsg)
{
    m_nThreadNo = nThreadNo;
    int ret = pthread_create(&m_tid, NULL, IThread::ThreadFunc, this);
    int err = errno;
    if(ret != 0) {
        strErrMsg = "Error: pthread_create failed, reason: ";
        strErrMsg.append(strerror(err));
        return false;
    }

    return true;
}

void IThread::start_thread() {
    m_nRunStatus = enRunning;
}

void IThread::stop_thread() {
    m_nRunStatus = enStopped;
}

const char* IThread::GetThreadStatus()
{
    switch (m_nRunStatus) {
        case enNoRunning: return "enNoRunning";
        case enRunning:   return "enRunning";
        case enStopping:  return "enStopping";
        case enStopped:   return "enStopped";
        default:         return "unknown status";
    }
}

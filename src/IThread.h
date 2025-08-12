#ifndef __ITHREAD_H__
#define __ITHREAD_H__

#include <iostream>
#include <string>
#include <pthread.h>
using namespace std;

enum {
    enNoRunning = 0, //未运行
    enRunning = 1, //正在运行
    enStopping = 2,//正在停止
    enStopped = 3, //已经停止
};

class IThread
{
public:
    IThread();
    virtual ~IThread();
    
public:
    static void* ThreadFunc(void* arg);
    virtual bool init(int nThreadNo, string& strErrMsg);
    void start_thread();
    void stop_thread();
    virtual void start() = 0;
    virtual const char* GetThreadStatus();

public:
    int m_nRunStatus;
    int m_nThreadNo;

protected:
    pthread_t m_tid;
};

#endif

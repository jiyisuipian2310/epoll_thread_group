#ifndef __INNERMSGTHREAD_H__
#define __INNERMSGTHREAD_H__
#include "IThread.h"
#include <pthread.h>
#include <boost/lockfree/queue.hpp>
#include <iostream>
#include <string>
using namespace std;

class IInnerMsg;
class AcceptMsgThread : public IThread {
public:
    AcceptMsgThread():m_msgqueue(50000) {}
    ~AcceptMsgThread() {}
    virtual void start();
    virtual void add_inner_message(IInnerMsg* pInnerMsg);
    virtual bool init(int nThreadNo, string strThreadName, string& strErrMsg);

protected:
    virtual IInnerMsg* GetInnerMsg();

protected:
    boost::lockfree::queue<IInnerMsg*> m_msgqueue;
    string m_strThreadName;
};

class ClientServerMsgThread : public AcceptMsgThread {
public:
    ClientServerMsgThread() {}
    ~ClientServerMsgThread() {}
};

class TimeoutDisconnectMsgThread : public AcceptMsgThread {
public:
    TimeoutDisconnectMsgThread() {}
    ~TimeoutDisconnectMsgThread() {}
};

class TimeoutLogRecordMsgThread : public AcceptMsgThread {
public:
    TimeoutLogRecordMsgThread() {}
    ~TimeoutLogRecordMsgThread() {}
};

#endif

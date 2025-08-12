#ifndef __LOCK_FREE_QUEUE_H__
#define __LOCK_FREE_QUEUE_H__

#include <string>
#include <iostream>
#include <boost/lockfree/queue.hpp>
using namespace std;

class LogLockFreeQueue {
public: 
    LogLockFreeQueue();
    ~LogLockFreeQueue();

    void push(const char* format, ...);
    bool pop(std::string& message);

private:
    boost::lockfree::queue<std::string*> m_logQueue;
};

#endif // __LOCK_FREE_QUEUE_H__
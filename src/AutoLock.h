#ifndef __AUTOLOCK_H__
#define __AUTOLOCK_H__

#include <pthread.h>

class AutoMutexLock {
public:
    AutoMutexLock(pthread_mutex_t* pLockObj): m_pLockObj(pLockObj) {
        if(m_pLockObj != NULL) pthread_mutex_lock(m_pLockObj);
    }

    ~AutoMutexLock() {
        if(m_pLockObj != NULL) pthread_mutex_unlock(m_pLockObj);
    }

private:
    pthread_mutex_t* m_pLockObj;
};

class AutoRWLock {
public:
    AutoRWLock(pthread_rwlock_t* pLockObj, bool bRLock = true): m_pLockObj(pLockObj) {
        if(m_pLockObj != NULL) {
            if(bRLock == true) 
                pthread_rwlock_rdlock(m_pLockObj);
            else  
                pthread_rwlock_wrlock(m_pLockObj);
        }
    }

    ~AutoRWLock() {
        if(m_pLockObj != NULL) pthread_rwlock_unlock(m_pLockObj);
    }
private:
    pthread_rwlock_t* m_pLockObj;
};

#endif

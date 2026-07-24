#include <thread.h>

using namespace Thread;

ThreadLock& ThreadLockManager::GetNewThreadLock() {
    if ( m_currentIndex > THREADLOCK_POOL_SIZE - 1 )
        throw ThreadLockPoolExceeded();
    return m_threadLocksPool[ m_currentIndex++ ];
}

namespace LIJ { ThreadLockManager& _ThreadLockManager = ThreadLockManager::Instance(); } // global instance
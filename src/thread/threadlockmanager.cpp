#include <thread.h>

using namespace Thread;

std::unique_ptr<ThreadLockManager> ThreadLockManager::s_pInstance = nullptr;

// get singleton instance
ThreadLockManager& ThreadLockManager::Instance() {
    if (!s_pInstance.get())
        s_pInstance = std::make_unique<ThreadLockManager>();
    return *s_pInstance;
};

ThreadLock& ThreadLockManager::GetNewThreadLock() {
    if ( m_currentIndex > THREADLOCK_POOL_SIZE - 1 )
        throw ThreadLockPoolExceeded();
    return m_threadLocksPool[ m_currentIndex++ ];
}

namespace LIJ { ThreadLockManager& _ThreadLockManager = ThreadLockManager::Instance(); } // global instance
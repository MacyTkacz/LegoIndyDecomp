#pragma once

#include <mutex>
#include <cstdint>
#include <stdexcept>
#include <chrono>

#define THREAD_LOCK_TIMEOUT 10s // normally defined in windows registry
#define THREADLOCK_POOL_SIZE 12

namespace Thread {

// light wrapper class for mutex to more closely resemble win32 critical sections behavior
class ThreadLock {
public:
    ThreadLock();
    ~ThreadLock();
    ThreadLock(const ThreadLock&) = delete;
    ThreadLock& operator=(const ThreadLock&) = delete;

    void Lock();
    void Unlock();
private:
    std::chrono::seconds m_timeout;
    std::timed_mutex m_mutex;
    bool m_isLocked;
};

// singleton class for distributing references to a pool of ThreadLock objects
class ThreadLockManager {
public:
    ThreadLock& GetNewThreadLock();

    //singleton management
    static ThreadLockManager& Instance();
private:
    static std::unique_ptr<ThreadLockManager> s_pInstance;

    uint8_t m_currentIndex = 0;
    std::array<ThreadLock,THREADLOCK_POOL_SIZE> m_threadLocksPool;
};

// EXCEPTIONS

class ThreadLockTimeout : public std::runtime_error {
public:
    ThreadLockTimeout() : std::runtime_error("Timeout reached while waiting for ThreadLock ownership") { };
};

class ThreadLockPoolExceeded : public std::runtime_error {
public:
    ThreadLockPoolExceeded() : std::runtime_error("ThreadLockManager 'GetNewThreadLock()' requests exceeded THREADLOCK_POOL_SIZE") { };
};

}; // namespace Thread

namespace LIJ { extern Thread::ThreadLockManager& _ThreadLockManager; } // global instance
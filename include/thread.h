#pragma once

#include <mutex>
#include <cstdint>
#include <stdexcept>
#include <chrono>

#define THREAD_LOCK_TIMEOUT 10s // normally defined in windows registry

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

class ThreadLockTimeout : public std::runtime_error {
public:
    ThreadLockTimeout() : std::runtime_error("Timeout reached while waiting for ThreadLock ownership") { };
};

}; // namespace Thread
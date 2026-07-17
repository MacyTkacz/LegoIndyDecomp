#include <thread.h>
#include <iostream>
#include <chrono>
#include <cstdint>

using namespace std::chrono_literals;
using namespace Thread;

ThreadLock::ThreadLock() {
    m_isLocked = false;
    m_timeout = std::chrono::seconds(THREAD_LOCK_TIMEOUT); 
}

ThreadLock::~ThreadLock() {
    Unlock();
}

void ThreadLock::Lock() {
    if (!m_mutex.try_lock_for(m_timeout))
        throw ThreadLockTimeout();
    m_isLocked = true;
}

void ThreadLock::Unlock() {
    if (!m_isLocked)
        return;
    m_mutex.unlock();
    m_isLocked = false;
}
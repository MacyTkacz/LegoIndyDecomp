#include <iostream>
#include <cstring>
#include <thread.h>

using namespace Thread;

ThreadLockManager& tlm = ThreadLockManager::Instance();

int main() {
	ThreadLock& a1 = tlm.GetNewThreadLock();
	a1.Lock();
	ThreadLock& a2 = tlm.GetNewThreadLock();
	a2.Lock();
	a2.Unlock();
	a1.Unlock();
	return 0;
}
#include <iostream>
#include <cstring>
#include <thread.h>

using namespace Thread;

int main() {
	ThreadLock& a1 = LIJ::_ThreadLockManager.GetNewThreadLock();
	a1.Lock();
	ThreadLock& a2 = LIJ::_ThreadLockManager.GetNewThreadLock();
	a2.Lock();
	a2.Unlock();
	a1.Unlock();
	return 0;
}
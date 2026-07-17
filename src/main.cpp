#include <filesystem.h>
#include <iostream>
#include <thread.h>

int main() {

	Thread::ThreadLock tl;
	tl.Lock();
	try {
		tl.Lock();
	}
	catch(const Thread::ThreadLockTimeout& e) {
		std::cerr << e.what() << '\n';
	}
	
	tl.Unlock();

	return 0;

}
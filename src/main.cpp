#include <iostream>
#include <cstring>

#include <thread.h>
#include <filesystem.h>

Thread::ThreadLockManager& tlm = LIJ::_ThreadLockManager.Instance();
FileSystem::FileHandlesManager& fhm = LIJ::_FileHandlesManager.Instance();

int main() {
	std::cout << "hello, world!\n";
	return 0;
}
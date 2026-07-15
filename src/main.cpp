#include <filesystem.h>
#include <iostream>

// debugging
#include <cerrno>
#include <cstring>

void printErrorString() {
#ifdef _WIN32
	std::cout << "error code: " << GetLastError() << std::endl;
#else
	std::cout << std::strerror(errno) << std::endl;
#endif
}

int main() {

	using namespace FileSystem;

	constexpr const char* path = "include/filesystem.h";

	auto accessType = FileAccessType::READ;
	auto shareType = FileShareType::READ;
	auto createMode = FileCreateMode::_OPEN_EXISTING; 
	auto attributes = FileAttribute::NORMAL;

	std::unique_ptr<File> pFile = GetFile(path,accessType,shareType,createMode,attributes);

	if (!pFile) {
		std::cout << "file initialization failed :(" << std::endl;
		printErrorString();
		return 1;
	}

	char buff[256];
	memset(buff,0,256);
	pFile->SetPointer(FilePosition::START,nullptr);
	pFile->Read(&buff,255);

	std::cout << buff << std::endl;

	return 0;

}
#include <compat/filesystem/filesystem.h>
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

	const char* path = "file.txt";
	auto accessType = FileAccessType::READ | FileAccessType::WRITE;
	auto shareType = FileShareType::READ | FileShareType::WRITE;
	auto createMode = FileCreateMode::_CREATE_NEW; 
	auto attributes = FileAttribute::NORMAL;

	std::shared_ptr<File> pFile = GetFile(path,accessType,shareType,createMode,attributes);

	if (!pFile) {
		std::cout << "file creation failed :(" << std::endl;
		printErrorString();
		return 1;
	}

	std::string s("hello, world!");
	pFile->Write(s.data(),s.length());
	pFile->Save();

	char buff[14];
	pFile->SetPointer(FilePosition::START,nullptr);
	pFile->Read(&buff,13,nullptr);

	std::cout << buff << std::endl;

	return 0;

}
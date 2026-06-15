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

	const char* path = "file.txt";
	auto accessType = FileSystem::FileAccessType::READ | FileSystem::FileAccessType::WRITE;
	auto shareType = FileSystem::FileShareType::READ | FileSystem::FileShareType::WRITE;
	auto createMode = FileSystem::FileCreateMode::_CREATE_ALWAYS; 
	auto attributes = FileSystem::FileAttribute::NORMAL;

	std::shared_ptr<FileSystem::File> pFile = FileSystem::GetFile(path,accessType,shareType,createMode,attributes);

	if (!pFile) {
		std::cout << "file creation failed :(" << std::endl;
		printErrorString();
		return 1;
	}

	std::string s("hello, world!");
	pFile->Write(s.data(),s.length(),nullptr);
	pFile->Save();

	char buff[14];
	pFile->SetPointer(FileSystem::FilePosition::START,nullptr);
	pFile->Read(&buff,13,nullptr);

	std::cout << buff << std::endl;

	return 0;

}
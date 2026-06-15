#include <compat/filesystem/filesystem.h>
#include <iostream>

// debugging
#include <cerrno>
#include <cstring>
void printErrorString() { std::cout << std::strerror(errno) << std::endl; }

int main() {

	const char* path = "file.txt";
	uint8_t accessType = static_cast<uint8_t>(FileSystem::FileAccessType::READ|FileSystem::FileAccessType::WRITE);
	uint8_t shareType = static_cast<uint8_t>(FileSystem::FileShareType::READ|FileSystem::FileShareType::WRITE);
	uint8_t createMode = static_cast<uint8_t>(FileSystem::FileCreateMode::_CREATE_ALWAYS); 
	uint64_t attributes = FileSystem::FileAttribute::NORMAL;

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
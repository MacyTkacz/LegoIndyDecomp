#include <compat/filesystem/filesystem.h>
#include <iostream>
#include <string>

int main() {

	const char* path = "file.txt";
	uint8_t accessType = static_cast<uint8_t>(FileSystem::FileAccessType::READ|FileSystem::FileAccessType::WRITE);
	uint8_t shareType = static_cast<uint8_t>(FileSystem::FileShareType::READ);
	uint8_t createMode = static_cast<uint8_t>(FileSystem::FileCreateMode::_CREATE_ALWAYS); 
	uint64_t attributes = FileSystem::FileAttribute::NORMAL;

	std::shared_ptr<FileSystem::File> pFile = FileSystem::GetFile(path,accessType,shareType,createMode,attributes);
	std::string s("hello, world!");
	pFile->Write(s.data(),s.length(),0);
	pFile->Save();

	return 0;

}
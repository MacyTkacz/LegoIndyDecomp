#include <compat/filesystem.h>
#include <iostream>
#include <string>

int main() {

	const char* path = "file.txt";
	uint8_t accessType = static_cast<uint8_t>(FileSystem::FileAccessType::WRITE);
	uint8_t shareType = static_cast<uint8_t>(FileSystem::FileShareType::NONE);
	uint8_t createMode = FILECREATEMODE(CREATE_NEW);
	uint64_t attributes = FileSystem::FileAttribute::NORMAL;
	std::shared_ptr<FileSystem::File> pFile = FileSystem::GetFile(path,accessType,shareType,createMode,attributes);
	// std::cout << std::to_string(errno) << std::endl;
	return 0;

}
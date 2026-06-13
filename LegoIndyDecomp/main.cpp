#include <compat/filesystem.h>

int main() {

	const char* path = "../file.txt";
	FileSystem::FileAccessType accessType = FileSystem::FileAccessType::READ;
	FileSystem::FileShareType shareType = FileSystem::FileShareType::NONE;
	FileSystem::FileCreateMode createMode = FileSystem::FileCreateMode::OPEN_EXISTING;
	uint64_t attributes = FileSystem::FileAttribute::NORMAL;
	std::shared_ptr<FileSystem::File> pFile = FileSystem::GetFile(path,accessType,shareType,createMode,attributes);
	return 0;

}


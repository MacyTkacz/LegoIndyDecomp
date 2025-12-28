#include "main.h"
#include <iostream>
#include <fileio/fileio.h>

int main() {

	HEAP_INIT(16);

	char* fpath = (char*)GetOnHeap<char[64]>();
	HEAP_HOOK(fpath);
	strcpy_s(fpath, 64, "C:/Users/thoma/OneDrive/Desktop/shaders.h");

	FileIOManager* fiom = FileIOManager::Instance();
	if (fiom->CreateFileHandle((LPCSTR)fpath, FileAccessType::READ) == -1) {
		std::cerr << "failed to create file handle" << std::endl;
		return 0;
	}

	FileHandleContainer* pFileHandleContainer = (FileHandleContainer*)GetOnHeap<FileHandleContainer>();
	HEAP_HOOK(pFileHandleContainer);

	pFileHandleContainer->fileHandleIndex = 0;
	pFileHandleContainer->fileDataLength = 1024;

	fiom->Read(pFileHandleContainer);

	fiom->CloseFileHandle(0);

	HEAP_FREE();

	return 0;

}


#include "main.h"
#include <iostream>
#include <fileio/fileio.h>

int main() {

	HEAP_INIT(16);

	char* fpath = (char*)GetOnHeap<char[64]>();
	HEAP_HOOK(fpath);
	strcpy_s(fpath, 64, "C:/Users/thoma/OneDrive/Desktop/shaders.h");

	FileIOManager* fiom = FileIOManager::Instance();
	if (fiom->CreateFileHandle((LPCSTR)fpath,FileAccessType::READ) == -1)
		std::cout << "failed to create file handle" << std::endl;

	char* buff = (char*)GetOnHeap<char[4096]>();
	HEAP_HOOK(buff);

	fiom->Read(0, buff, 4096);

	fiom->CloseFileHandle(0);

	std::cout << buff << std::endl;

	HEAP_FREE();

	return 0;

}


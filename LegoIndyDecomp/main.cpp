#include "main.h"
#include <iostream>
#include <fileio/fileio.h>

int main() {

	HEAP_INIT(16);

	char* fpath = (char*)GetOnHeap<char[64]>();
	strcpy_s(fpath, 64, "C:/Users/thoma/OneDrive/Desktop/shaders.h");

	FileIOManager* fiom = FileIOManager::Instance();
	if (!fiom->CreateFileHandle((LPCSTR)fpath,FileAccessType::READ))
		std::cout << "failed to create file handle" << std::endl;

	HEAP_FREE();

	return 0;

}


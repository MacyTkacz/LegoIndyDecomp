#include "main.h"
#include <iostream>
#include <strings/lz2k.h>

int main() {

	HEAP_INIT(16);

	char* buff = (char*)GetOnHeap<char[12]>();
	HEAP_HOOK(buff);
	memcpy(buff, "LZ2K\0\x4\0\0\0\0\x1\0", 12);

	std::cout << LZ2K_DecodeFileSize(buff) << std::endl;

	HEAP_FREE();

	return 0;

}


#include "main.h"
#include <iostream>
#include <strings/lz2k.h>

int main() {

	HEAP_INIT(16);

	char* buff = (char*)GetOnHeap<char[32]>();
	HEAP_HOOK(buff);
	memcpy(buff, "LZ2K\0\x4\0\0", 8);

	std::cout << LZ2K_DecodeChunkSize(buff) << std::endl;

	HEAP_FREE();

	return 0;

}


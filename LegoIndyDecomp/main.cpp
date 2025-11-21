#include "main.h"
#include <string.h>
#include <strings/hash.h>
#include <iostream>

int main() {

	HEAP_INIT(16);

	char fname[32];
	memset(fname, 0, 32);
	strcpy_s(fname, 32, "files/asset.gsc");

	Hash* hashArray = (Hash*)GetOnHeap<Hash[3]>();
	HEAP_HOOK(hashArray);

	char* str1 = (char*)GetOnHeap<char[32]>();
	HEAP_HOOK(str1);
	strcpy_s(str1, 32, "redherring");
	hashArray[0].str = str1;
	hashArray[0].nextOnMatch = 0;
	hashArray[0].nextOnNonmatch = 1;

	char* str2 = (char*)GetOnHeap<char[32]>();
	HEAP_HOOK(str2);
	strcpy_s(str2, 32, "files");
	hashArray[1].nextOnMatch = 1;
	hashArray[1].nextOnNonmatch = 2;
	hashArray[1].str = str2;

	char* str3 = (char*)GetOnHeap<char[32]>();
	HEAP_HOOK(str3);
	strcpy_s(str3, 32, "asset.gsc");
	hashArray[2].nextOnMatch = -123;
	hashArray[2].nextOnNonmatch = 3;
	hashArray[2].str = str3;

	Hashes* pHashesStruct = (Hashes*)GetOnHeap<Hashes>();
	HEAP_HOOK(pHashesStruct);

	pHashesStruct->hashArray = hashArray;

	std::cout << func(pHashesStruct, fname) << std::endl;

	HEAP_FREE();

	return 0;

}

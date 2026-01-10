#include <iostream>
#include <utils.h>
#include <fileio/fileio.h>

int main() {

	HEAP_INIT(16);

	FileIOManager* fiom = FileIOManager::Instance();
	if (!fiom)
		return 1;

	char* path = (char*)GetOnHeap<char[64]>();
	strncpy(path, "/shaders",64);
	HEAP_HOOK(path);

	char* dir1 = (char*)GetOnHeap<char[64]>();
	strncpy(dir1,"", 64);
	HEAP_HOOK(dir1);

	Hash* pHashArray = (Hash*)GetOnHeap<Hash[2]>();
	pHashArray[0].str = dir1;
	pHashArray[0].nextOnMatch = 1;
	pHashArray[1].str = dir1;
	pHashArray[1].nextOnMatch = -1;
	HEAP_HOOK(pHashArray);

	Hashes* pHashesStruct = (Hashes*)GetOnHeap<Hashes>();
	pHashesStruct->hashArray = nullptr;
	pHashesStruct->numOfStringHashIndexPairs = 1;
	HEAP_HOOK(pHashesStruct);

	fiom->SomeLargeFileReadingFunction(pHashesStruct, path, FileAccessType::READ);

	std::cout << "hello world!" << std::endl;

	HEAP_FREE();
	return 0;

}


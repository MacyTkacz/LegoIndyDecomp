#include <iostream>
#include <utils.h>
#include <fileio/fileio.h>

int main() {

	HEAP_INIT(16);

	// FileIOManager* fiom = FileIOManager::Instance();
	// if (!fiom)
	// 	return 1;

	// char* textBuffer = (char*)GetOnHeap<char[1024]>();
	// HEAP_HOOK(textBuffer);

	// char* fname = (char*)GetOnHeap<char[64]>();
	// strncpy(fname,"C:/Users/thoma/OneDrive/Desktop/shaders.h",64);
	// HEAP_HOOK(fname);

	// char* pairsbuff = (char*)GetOnHeap<char[64]>();
	// strncpy(pairsbuff,"C:\\Users\\thoma\\OneDrive\\Desktop\\shaders.h\x00\x00\x01",64);
	// HEAP_HOOK(pairsbuff);

	// DATParser* pDATParser = (DATParser*)GetOnHeap<Hashes>();
	// memset(pDATParser,0,sizeof(Hashes));
	// pDATParser->numOfStringHashIndexPairs = 1;
	// pDATParser->stringHashIndexPairsBuffer = pairsbuff;
	// HEAP_HOOK(pDATParser);

	// fiom->SetHashesStruct(pDATParser);

	// std::cout << fiom->TopLevelFileReadingFunction(fname,textBuffer,1024) << std::endl;

	HEAP_FREE();
	return 0;

}


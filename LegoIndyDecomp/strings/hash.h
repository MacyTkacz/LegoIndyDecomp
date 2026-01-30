#ifndef LEGOINDY_HASH_H
#define LEGOINDY_HASH_H

#include "types.h"
#include <fileio/types.h>

// ===================== STRUCTS =====================

// class Hashes {
// public:

//     int LinkAvailableFilePointerContainer(int hashesStructIndex);

//     int status;
//     int SomeSixteenCount;
//     SomeSixteen *SomeSixteenArray;
//     int *hashes;
//     int numOfStringHashIndexPairs;
//     char stringHashIndexPairsBufferSize;
//     char *stringHashIndexPairs;
//     char pad1[4];
//     FilePointerContainer filePointerContainersArray[8];
//     __int16 someInt16;
//     __int16 fileAccessType;
//     char pad2[8];
//     char *DATfileName;
//     char DATfileNameBuffer[];
// };

// ===================== FUNCTIONS =====================

// gets the index of str's hash, if any
int GetHashIndex(DATParser* pDATParser, char* str);

int GetFormattedHashIndex(DATParser* pDATParser, char* fpath);

#endif // LEGOINDY_HASH_H
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

// calculates the hash of path_in
// checks whether the hash exists in struct's hashes array
// if not, checks whether path_in is in struct's stringsDataBuffer array
// if so, adds the hash to the hashes array
// returns the index of path_in's hash
int GetHashIndex(DATParser* pDATParser, char* path_in);

int GetFileDataIndex(DATParser* pDATParser, char* fpath);

#endif // LEGOINDY_HASH_H
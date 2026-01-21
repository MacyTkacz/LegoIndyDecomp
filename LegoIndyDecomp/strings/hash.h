#ifndef LEGOINDY_HASH_H
#define LEGOINDY_HASH_H

#include "types.h"
#include <fileio/types.h>

// ===================== STRUCTS =====================

class Hashes {
public:

    int LinkAvailableFilePointerContainer(int hashesStructIndex);

    int status;
    int SomeStructIndex;
    SomeStruct *SomeStructArray;
    int hashCount;
    Hash *hashArray;
    int status2;
    int int3;
    int *hashes;
    int numOfStringHashIndexPairs;
    char pad2[4];
    char *stringHashIndexPairs;   // exact length variable?
    char pad3[4];
    FilePointerContainer filePointerContainersArray[8];
    __int16 someInt16;
    __int16 fileAccessType;
    int int4;
    char pad5[4];
    char *DATfileName;
    char DATfileNameBuffer[];
};

// ===================== FUNCTIONS =====================

// calculates the hash of path_in
// checks whether the hash exists in struct's hashes array
// if not, checks whether path_in is in struct's stringsDataBuffer array
// if so, adds the hash to the hashes array
// returns the index of path_in's hash
int __cdecl GetHashIndex(Hashes* pHashesStruct, char* path_in);

int __cdecl GetFileDataIndex(Hashes* pHashesStruct, char* fpath);

#endif // LEGOINDY_HASH_H
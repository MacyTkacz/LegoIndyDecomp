#ifndef LEGOINDY_HASH_H
#define LEGOINDY_HASH_H

#include <fileio/types.h>

// ===================== STRUCTS =====================

struct Hash {
	__int16 nextOnMatch;
	__int16 nextOnNonmatch;
	char* str;
};

class Hashes {
public:

    int LinkAvailableFilePointerContainer(int hashesStructIndex);

    int status;
    int someIndex;
    SomeStruct *SomeStructArray;
    char pad1[4];
    Hash *hashArray;
    int int2;
    int int3;
    int *hashes;
    int numOfStringHashIndexPairs;
    char pad2[4];
    char *stringHashIndexPairsBuffer;   // exact length variable?
    char pad3[4];
    FilePointerContainer filePointerContainersArray[8];
    char pad4[2];
    __int16 fileAccessType;
    int int4;
    char pad5[4];
    char *fileName;
    char someString4[16];
    int int5;
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
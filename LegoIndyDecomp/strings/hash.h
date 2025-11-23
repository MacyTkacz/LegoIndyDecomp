#ifndef LEGOINDY_HASH_H
#define LEGOINDY_HASH_H

// ===================== STRUCTS =====================

struct Hash {
	__int16 nextOnMatch;
	__int16 nextOnNonmatch;
	char* str;
};

struct Hashes {
	int someStatus; // 0x00
	int numOfHashes; // 0x04
	char padding1[8]; // 0x08
	Hash* hashArray; // 0x10
	char padding[8]; // 0x14
	int* hashes; // 0x1C
	int numOfStringHashIndexPairs; // 0x20
	char padding3[4]; // 0x24
	char* stringHashIndexPairsBuffer; // 0x28
	char padding4[148]; // 0x2C
	const char* fileName; // 0xBC
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
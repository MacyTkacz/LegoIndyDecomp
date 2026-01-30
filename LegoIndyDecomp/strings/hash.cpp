#include "hash.h"
#include <strings/strings.h>
#include <strings/std.h>
#include <fileio/fileio.h>

int GetHashIndex(DATParser* pDATParser, char *str) {

	char* currentStrOffset = str;
	char currentChar = *str;

	// magic numbers found in code
	int magic1 = -2128831035;
	int magic2 = 1677619;

	// calculates the path hash, possibly using FNV-1A
	int strHash = magic1;
	for (;*currentStrOffset; currentChar = *currentStrOffset) {
		++currentStrOffset;
		strHash = magic2 * ( strHash ^ currentChar );
	}

	// look for a pre-existing path hash match
	int numberOfHashes = pDATParser->SomeSixteenArray.size();
	int hashIndex = 0;
	if (numberOfHashes > 0) {
		for (int i = 0; i < pDATParser->hashes.size(); i++) {
			if (pDATParser->hashes[i] == strHash)	
				return i;
		}
	}

	int numberOfStringHashIndexPairs = pDATParser->numOfStringHashIndexPairs;
	if (!numberOfStringHashIndexPairs)
		return -1;

	char* currentBufferOffset = pDATParser->stringHashIndexPairs.data();
	for (int _ = 0; _ < numberOfStringHashIndexPairs; ++_) {

		// if the buffer string is identical to str, return the subsequent short
		currentStrOffset = str;
		if (currentBufferOffset) {
			while (true) {
				char currentBufferChar = currentStrOffset[currentBufferOffset - str];
				char currentPathChar = *currentStrOffset;
				if (currentBufferChar != *currentStrOffset)
					break;
				currentStrOffset++;
				if ( !currentBufferChar || !currentPathChar ) {
					int stringLength = 0;
					if (*currentBufferOffset) {
						do
							++stringLength;
						while ( currentBufferOffset[stringLength] );
					}
					// aligns to first even address after string's null-terminator and returns short
					char* offset = &currentBufferOffset[stringLength+1];
					if ( reinterpret_cast<uintptr_t>( offset ) & 1 )
						offset++;
					return *reinterpret_cast<short*>( offset );
				}
			}
		}

		// otherwise, skip over this string + short pair
		int length = 0;
		if ( *currentBufferOffset ) {
			do
				++length;
			while ( currentBufferOffset[length] );
		}
		currentBufferOffset += length + 3;
		if ( reinterpret_cast<uintptr_t>( currentBufferOffset ) & 1 )
			currentBufferOffset++;

	}

	return -1;

}

int __cdecl GetFileDataIndex(DATParser* pDATParser, char* fpath) {
	return 0;
}

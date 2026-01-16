#include "hash.h"
#include <strings/strings.h>
#include <strings/std.h>
#include <fileio/fileio.h>

int __cdecl GetHashIndex(Hashes *pHashesStruct, char *path) {

  int *pCurrentHash; // ecx
  char *currentPathOffset2; // esi
  char currentBufferChar; // al
  char current_path_char; // dl
  int stringLength; // eax
  char *finalStringOffset; // eax
  int currentStringLength; // eax

  char* currentPathOffset = path;
  char currentChar = *path;

  // generates a string hash, possibly using FNV-1a
  int pathHash;
  for ( pathHash = -2128831035; *currentPathOffset; currentChar = *currentPathOffset ) {
    ++currentPathOffset;
    pathHash = 1677619 * (pathHash ^ currentChar);
  }

  int hashIndex = 0;
  int numberOfHashes = pHashesStruct->someIndex;

  if ( numberOfHashes <= 0 ) {
LABEL_7:
    int numberOfStrings = pHashesStruct->numOfStringHashIndexPairs;
    if ( numberOfStrings )
    {
      char* currentStringOffset = pHashesStruct->stringHashIndexPairsBuffer;
      for ( int _ = 0; _ < numberOfStrings; ++_ )
      {
        currentPathOffset2 = path;
        if ( currentStringOffset )
        {
          while ( 1 )
          {
            currentBufferChar = currentPathOffset2[currentStringOffset - path];
            current_path_char = *currentPathOffset2;
            if ( currentBufferChar > *currentPathOffset2 || currentBufferChar < *currentPathOffset2 )
              break;
            ++currentPathOffset2;
            if ( !currentBufferChar || !current_path_char )
            {
              stringLength = 0;
              if ( *currentStringOffset )
              {
                do
                  ++stringLength;
                while ( currentStringOffset[stringLength] );
              }
              // finds address after some_str's null-terminator
              // aligns to even address
              // and returns int16 value at that address
              finalStringOffset = &currentStringOffset[stringLength + 1];
              if ( (reinterpret_cast<uintptr_t>(currentStringOffset) & 1) != 0 )
                ++finalStringOffset;
              return *(__int16 *)finalStringOffset;// that string's hash index?
            }
          }
        }
        currentStringLength = 0;
        if ( *currentStringOffset )
        {
          do
            ++currentStringLength;
          while ( currentStringOffset[currentStringLength] );
        }
        // proceeds to next string in stringsData
        // each string is some length of chars (null-terminated)
        // followed by 2 bytes of data, the last of which may be returned
        // a third byte may exist for 2-byte alignment
        currentStringOffset += currentStringLength + 3;
        if ( (reinterpret_cast<uintptr_t>(currentStringOffset) & 1) != 0 )
          ++currentStringOffset;
      }
    }
    return -1;
  }
  else
  {
    pCurrentHash = pHashesStruct->hashes;
    while ( *pCurrentHash != pathHash )
    {
      ++hashIndex;
      ++pCurrentHash;
      if ( hashIndex >= numberOfHashes )
        goto LABEL_7;
    }
  }
  return hashIndex;
}

int __cdecl GetFileDataIndex(Hashes* pHashesStruct, char* fpath) {

	Hash* hashArray;
	Hash* currentHash;

	char* startOfFpath;
	char* bufferOffset;
	char* currentDirectoryName;
	char* currentPathOffset;
	char* nextDirectoryName;

	char currentChar;
	char path[256];

	int fpathToBufferDelta;
	int hashIndex;
	__int16 hashValue;

  strcpy(path,fpath + (*fpath == '@' ? 4 : 0));

	MakePathUniform(0, path);

	hashArray = pHashesStruct->hashArray;
	currentDirectoryName = path;

	if (!hashArray)
		return GetHashIndex(pHashesStruct, path);

	hashIndex = hashArray->nextOnMatch;
	currentPathOffset = GetStringStartingWith(path, "\\");
	if (currentPathOffset)
		*currentPathOffset = 0;

	while (1) {

		currentHash = &pHashesStruct->hashArray[hashIndex];
		if (!_strcmpi(currentHash->str, currentDirectoryName))
			break;
		hashIndex = currentHash->nextOnNonmatch;

VALIDATE_hashIndex:
		if (!hashIndex)
			return -1;

	}

	if (currentPathOffset) {

		hashIndex = currentHash->nextOnMatch;
		currentDirectoryName = currentPathOffset + 1;
		nextDirectoryName = GetStringStartingWith(currentPathOffset + 1, "\\");
		currentPathOffset = nextDirectoryName;

		if (nextDirectoryName)
			*nextDirectoryName = 0;

		goto VALIDATE_hashIndex;
		
	}

	hashValue = pHashesStruct->hashArray[hashIndex].nextOnMatch;
	if (hashValue <= 0)
		return -hashValue;

	return -1;

}

// attempts to set the fileHandleIndex of one of 8 available FilePointerContainers from FilePointerContainersArray
int Hashes::LinkAvailableFilePointerContainer(int hashesStructIndex) {

	FileIOManager* fiom = FileIOManager::Instance();
	if (!fiom)
		return -1;

	RawEnterCriticalSection( fiom->GetCriticalSectionIndex() );

	int availableFilePointerContainerIndex = 0;
	for (int i = 0; filePointerContainersArray[i].fileHandleIndex != -1; i++) {
		if (++availableFilePointerContainerIndex >= 8) {
			availableFilePointerContainerIndex = -1;
			goto EXIT;
		}
	}
	filePointerContainersArray[availableFilePointerContainerIndex].fileHandleIndex = hashesStructIndex;

EXIT:
	RawLeaveCriticalSection( fiom->GetCriticalSectionIndex() );
	return availableFilePointerContainerIndex;

}
#include "hash.h"
#include <strings/strings.h>
#include <strings/std.h>

int __cdecl GetHashIndex(Hashes *pHashesStruct, char *path_in) {
  char *current_path_offset; // ecx
  char current_char; // al
  int path_hash; // edx
  int number_of_hashes; // esi
  int hash_index; // eax
  int *pCurrentHash; // ecx
  int num_of_strings; // ebp
  char *current_str_offset; // ecx
  int _; // ebx
  char *current_path_offset_2; // esi
  char current_buff_char; // al
  char current_path_char; // dl
  int len_str; // eax
  char *final_str_offset; // eax
  int current_str_length; // eax

  current_path_offset = path_in;
  // generates a string hash, possibly using FNV-1a
  current_char = *path_in;
  for ( path_hash = -2128831035; *current_path_offset; current_char = *current_path_offset ) {
    ++current_path_offset;
    path_hash = 1677619 * (path_hash ^ current_char);
  }
  number_of_hashes = pHashesStruct->someIndex;
  hash_index = 0;
  if ( number_of_hashes <= 0 ) {
LABEL_7:
    num_of_strings = pHashesStruct->numOfStringHashIndexPairs;
    if ( num_of_strings )
    {
      current_str_offset = pHashesStruct->stringHashIndexPairsBuffer;
      for ( _ = 0; _ < num_of_strings; ++_ )
      {
        current_path_offset_2 = path_in;
        if ( current_str_offset )
        {
          while ( 1 )
          {
            current_buff_char = current_path_offset_2[current_str_offset - path_in];
            current_path_char = *current_path_offset_2;
            if ( current_buff_char > *current_path_offset_2 || current_buff_char < *current_path_offset_2 )
              break;
            ++current_path_offset_2;
            if ( !current_buff_char || !current_path_char )
            {
              len_str = 0;
              if ( *current_str_offset )
              {
                do
                  ++len_str;
                while ( current_str_offset[len_str] );
              }
              // finds address after some_str's null-terminator
              // aligns to even address
              // and returns int16 value at that address
              final_str_offset = &current_str_offset[len_str + 1];
              if ( ((unsigned __int8)final_str_offset & 1) != 0 )
                ++final_str_offset;
              return *(__int16 *)final_str_offset;// that string's hash index?
            }
          }
        }
        current_str_length = 0;
        if ( *current_str_offset )
        {
          do
            ++current_str_length;
          while ( current_str_offset[current_str_length] );
        }
        // proceeds to next string in stringsData
        // each string is some length of chars (null-terminated)
        // followed by 2 bytes of data, the last of which may be returned
        // a third byte may exist for 2-byte alignment
        current_str_offset += current_str_length + 3;
        if ( ((unsigned __int8)current_str_offset & 1) != 0 )
          ++current_str_offset;
      }
    }
    return -1;
  }
  else
  {
    pCurrentHash = pHashesStruct->hashes;
    while ( *pCurrentHash != path_hash )
    {
      ++hash_index;
      ++pCurrentHash;
      if ( hash_index >= number_of_hashes )
        goto LABEL_7;
    }
  }
  return hash_index;
}

int __cdecl GetFileDataIndex(Hashes* pHashesStruct, char* fpath) {

	Hash* hashArray;
	Hash* currentHash;

	char* startOfFpath;
	char* bufferOffset;
	char* currentDirectoryName;
	char* subName;
	char* currentPathOffset;
	char* nextDirectoryName;

	char currentChar;
	char path[256];

	int fpathToBufferDelta;
	int hashIndex;
	__int16 hashValue;

	startOfFpath = fpath;
	if (*fpath == '@')
		startOfFpath += 4;
	bufferOffset = path;

	if (startOfFpath) {
		currentChar = *startOfFpath;
		if (*startOfFpath) {
			fpathToBufferDelta = startOfFpath - path;
			do {
				*bufferOffset = currentChar;
				currentChar = (bufferOffset++)[fpathToBufferDelta + 1];
			}
			while (currentChar);
		}
	}

	*bufferOffset = 0;

	MakePathUniform(0, path);

	hashArray = pHashesStruct->hashArray;
	currentDirectoryName = path;

	if (!hashArray)
		return GetHashIndex(pHashesStruct, path);

	hashIndex = hashArray->nextOnMatch;
	subName = GetStringStartingWith(path, "\\");
	currentPathOffset = subName;
	if (subName)
		*subName = 0;

	while (1) {

		currentHash = &pHashesStruct->hashArray[hashIndex];
		if (!_strcmpi(currentHash->str, currentDirectoryName))
			break;
		hashIndex = currentHash->nextOnNonmatch;

VALIDATE_HASH_INDEX:
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

		goto VALIDATE_HASH_INDEX;
		
	}

	hashValue = pHashesStruct->hashArray[hashIndex].nextOnMatch;
	if (hashValue <= 0)
		return -hashValue;

	return -1;

}

#ifndef LEGOINDY_FILEIO_TYPES_H
#define LEGOINDY_FILEIO_TYPES_H

#include <Windows.h>
#include <strings/types.h>

class DATParser;

enum FileResourceType { INVALID, FILEHANDLECONTAINER, FILEBUFFERCONTAINER, FILEPOINTERINFO };

struct FilePointerContainer {
	int fileHandleID;
	int fileHandleIndex;
	LARGE_INTEGER filePointerPosition;
};

enum FileType {
	UNCOMPRESSED,
	TYPE2,
	LZ2K
};

struct SomeStruct {
    int someIndex;
	Hash* hashArray;
	char pad[8];
};

struct SomeSixteen {
	int chunkNumber; // nth 256-byte memory chunk the data begins in
	int int2;
	int int3;
	int int4;
};

#endif // LEGOINDY_FILEIO_TYPES_H
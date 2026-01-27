#ifndef LEGOINDY_FILEIO_TYPES_H
#define LEGOINDY_FILEIO_TYPES_H

#include <Windows.h>
#include <strings/types.h>

enum FileResourceType { INVALID, FILEHANDLECONTAINER, FILEBUFFERCONTAINER, FILEPOINTERINFO };

struct FilePointerContainer {
	int fileHandleID;
	int fileHandleIndex;
	LARGE_INTEGER filePointerPosition;
};

enum FileType {
	TYPE1,
	TYPE2,
	LZ2K
};

struct SomeStruct {
    int someIndex;
	Hash* hashArray;
	char pad[8];
};

#endif // LEGOINDY_FILEIO_TYPES_H
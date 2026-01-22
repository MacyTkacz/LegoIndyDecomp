#ifndef LEGOINDY_FILEIO_TYPES_H
#define LEGOINDY_FILEIO_TYPES_H

#include <Windows.h>
#include <strings/types.h>

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
    int hashCount;
	Hash hashArray[];
};

#endif // LEGOINDY_FILEIO_TYPES_H
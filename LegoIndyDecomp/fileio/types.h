#ifndef LEGOINDY_FILEIO_TYPES_H
#define LEGOINDY_FILEIO_TYPES_H

#include <Windows.h>

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
    int someNum;
    int fileDataSize1;
    int fileDataSize2;
    FileType fileType;
};

#endif // LEGOINDY_FILEIO_TYPES_H
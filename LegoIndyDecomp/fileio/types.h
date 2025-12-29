#ifndef LEGOINDY_FILEIO_TYPES_H
#define LEGOINDY_FILEIO_TYPES_H

#include <Windows.h>

struct FilePointerContainer {
	int fileHandleID;
	int fileHandleIndex;
	LARGE_INTEGER filePointerPosition;
};

#endif // LEGOINDY_FILEIO_TYPES_H
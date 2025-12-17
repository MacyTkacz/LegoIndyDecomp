#ifndef LEGOINDY_FILEIO_H
#define LEGOINDY_FILEIO_H

#include <Windows.h>

// ===================== STRUCTS =====================

enum FileAccessType { READ, CREATE, MODIFY };

// ===================== CLASSES =====================

// singleton class that manages file IO
class FileIOManager {
public:
	static FileIOManager* Instance();

	int InitializeNewCriticalSection();
	int CreateFileHandle(LPCSTR fpath, FileAccessType fileAccessType);
	bool CloseFileHandle(int fileHandleIndex);
	int Write(int fileHandleIndex, LPCVOID lpBuffer, int numberOfBytesToWrite);
	int Read(int fileHandleIndex, LPCVOID lpBuffer, int numberOfBytesToRead);
	LARGE_INTEGER MoveFilePointer(int fileHandleIndex, LARGE_INTEGER distToMove, int moveMethod);
private:
	static inline FileIOManager* _instance = 0;

	static inline int NumberOfCriticalSections = -1;
	int CriticalSectionLockCount;
	CRITICAL_SECTION CriticalSectionsArray[12];
	HANDLE FileHandlesArray[32];
	int CriticalSectionIndex;
};

// ===================== FUNCTIONS =====================

#endif // LEGOINDY_FILEIO_H

#ifndef LEGOINDY_FILEIO_H
#define LEGOINDY_FILEIO_H

#include <Windows.h>

// ===================== STRUCTS =====================

enum FileAccessType { READ, CREATE, MODIFY };

// ===================== CLASSES =====================

// singleton class that manages file IO
class FileIOManager {
public:
	FileIOManager();
	static FileIOManager* Instance();

	int AdvanceCriticalSection();
	int CreateFileHandle(LPCSTR fpath, FileAccessType fileAccessType);
	bool CloseFileHandle(int fileHandleIndex);
	void CloseFileHandleID(int fileHandleID);
	int Write(int fileHandleIndex, LPVOID lpBuffer, int numberOfBytesToWrite);
	int Read(int fileHandleIndex, LPVOID lpBuffer, int numberOfBytesToRead);
	LARGE_INTEGER MoveFilePointer(int fileHandleIndex, LARGE_INTEGER distToMove, int moveMethod);
private:
	static inline FileIOManager* _instance = 0;

	static inline int CriticalSectionIndex = -1;
	int CriticalSectionLockCount;
	CRITICAL_SECTION* CriticalSectionsArray[14];
	HANDLE FileHandlesArray[32];
};

// ===================== FUNCTIONS =====================

#endif // LEGOINDY_FILEIO_H

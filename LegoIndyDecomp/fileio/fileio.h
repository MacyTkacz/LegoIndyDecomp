#ifndef LEGOINDY_FILEIO_H
#define LEGOINDY_FILEIO_H

#include <Windows.h>

// ===================== STRUCTS =====================

enum FileAccessType { READ, CREATE, MODIFY };

struct FileHandleContainer;
struct FileDataContainer {
	FileHandleContainer* pFileHandleContainer;
	int int1;
	char dataBuffer[1024];
};

struct FileHandleContainer {
	int fileHandleIndex;
	char pad1[4];
	LARGE_INTEGER filePointerPosition;
	char pad2[8];
	LARGE_INTEGER fileEndPosition;
	int fileDataLength;
	int positionIsUpToDate;
	void* pSomething;
	char pad3[4];
	FileAccessType fileAccessType;
	FileDataContainer* pFileDataContainer;
};

// ===================== CLASSES =====================

class FileBufferContainer {
public:
	char* textBuffer; // 0x0
	char* textBufferEnd; // 0x4
	char* filePointerPosition; // 0x8
	int bSomeBool; // 0xC
	int bIsInUse = 0; // 0x10
};

// singleton class that manages file IO
class FileIOManager {
public:
	FileIOManager();
	static FileIOManager* Instance();

	int AdvanceCriticalSection();
	int CreateFileHandle(LPCSTR fpath, FileAccessType fileAccessType);
	bool CloseFileHandle(int fileHandleIndex);
	void CloseResource(int resourceID);
	int FormatAvailableFileBufferContainer(char* buffer, int bufferSize, unsigned int someValue);
	int Write(int fileHandleIndex, LPVOID lpBuffer, int numberOfBytesToWrite);
	int Read(int fileHandleIndex, LPVOID lpBuffer, int numberOfBytesToRead);
	LARGE_INTEGER MoveFilePointer(int fileHandleIndex, LARGE_INTEGER distToMove, int moveMethod);

	static constexpr int GetFileBufferContainersCount() { return 20; }
private:
	static inline FileIOManager* _instance = 0;

	static inline int CriticalSectionIndex = -1;
	static inline int CriticalSectionLockCount = 0;
	CRITICAL_SECTION* CriticalSectionsArray[14];
	HANDLE FileHandlesArray[32];
	FileHandleContainer FileHandleContainersArray[32];
	FileBufferContainer FileBufferContainersArray[1024];
};

#endif // LEGOINDY_FILEIO_H

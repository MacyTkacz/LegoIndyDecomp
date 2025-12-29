#ifndef LEGOINDY_FILEIO_H
#define LEGOINDY_FILEIO_H

#include <Windows.h>
#include <strings/hash.h>

// ===================== STRUCTS =====================

enum FileAccessType { READ, CREATE, MODIFY };

struct FileHandleContainer;
struct FileDataContainer {
	FileHandleContainer* pFileHandleContainer;
	int lastWriteIndex; // lower number = written to earlier
	char dataBuffer[1024];
};

struct FileHandleContainer {
	int fileHandleIndex;
	char pad1[4];
	LARGE_INTEGER filePointerPosition;
	char pad2[8];
	LARGE_INTEGER fileEndPosition;
	int fileDataLength;
	int bCanFileBeReadNonsequentially; // if false, always read from FILE_BEGIN (I think?)
	void* pSomething;
	char pad3[4];
	FileAccessType fileAccessType;
	FileDataContainer* pFileDataContainer;
};

struct FilePointerInfo {
	Hashes* pHashesStruct;
	DWORD dw1;
	LARGE_INTEGER fileStartPosition;
	LARGE_INTEGER filePointerPosition;
	DWORD fileDataSize;
	DWORD fileDataSizeWhen0x28isNonzero;
	int filePointerContainerIndex;
	DWORD fileOffset2;
	DWORD dw3;
	DWORD dw4;
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

	// reads file data into an available FileDataContainer
	int Read(FileHandleContainer* pFileHandleContainer);
	int AdvanceCriticalSection();
	int CreateFileHandle(LPCSTR fpath, FileAccessType fileAccessType);
	bool CloseFileHandle(int fileHandleIndex);
	void CloseResource(int resourceID);
	int FormatAvailableFileBufferContainer(char* buffer, int bufferSize, unsigned int someValue);
	int SetFilePointer(int resourceID, LARGE_INTEGER distToMove, DWORD moveMethod);
	static constexpr int GetFileBufferContainersCount() { return 20; }

	// resource ID reference values
	const static int FileHandleContainersBase = 0;
	const static int FileBufferContainersBase = 1024;
	const static int FilePointerInfosBase = 2048;
	const static int MaximumValidResourceID = 4096;

private:
	static inline FileIOManager* _instance = 0;

	static inline int CriticalSectionIndex = -1;
	static inline int CriticalSectionLockCount = 0;
	static inline int FilesReadCounter = 0; // increments when Read is called

	CRITICAL_SECTION* CriticalSectionsArray[14];
	HANDLE FileHandlesArray[32];
	FileHandleContainer FileHandleContainersArray[32];
	FileBufferContainer FileBufferContainersArray[1024];
	FileDataContainer FileDataContainersArray[4];
	FilePointerInfo FilePointerInfoArray[1024];

	// wrappers for Windows api
	int RawWrite(int fileHandleIndex, LPVOID lpBuffer, int numberOfBytesToWrite);
	int RawRead(int fileHandleIndex, LPVOID lpBuffer, int numberOfBytesToRead);
	LARGE_INTEGER RawSetFilePointer(int fileHandleIndex, LARGE_INTEGER distToMove, int moveMethod);
};

#endif // LEGOINDY_FILEIO_H

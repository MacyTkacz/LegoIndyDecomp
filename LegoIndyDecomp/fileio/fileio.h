#ifndef LEGOINDY_FILEIO_H
#define LEGOINDY_FILEIO_H

#include <Windows.h>
#include <strings/hash.h>

// ===================== STRUCTS =====================

enum FileAccessType { READ, CREATE, MODIFY, OTHER };

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
	DWORD bIsInUse;
	DWORD dw3;
	DWORD dw4;
};

struct PathTypeInfo {
	int status1;
	char pad1[12];
	char separator;
};

struct FilePathContainer {
	PathTypeInfo pathTypeInfo;
	char pad1[35];
	char absolutePath[16];
	int pathLength;
	char pad2[4];
	char drivePrefix[16];
	char pad3[16];
	char someStr[32];
	char pad4[32];
	char relativePath[16];
	char pad5[368];
	int (*pathJoiningFunction)(FilePathContainer *, char *fpath_out, char *fpath_in, int size);
    int (*func2)(FilePathContainer *);
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
	FilePathContainer* GetFilePathContainerFromPath(char* fpath);
	int GetAvailableFilePointerInfoIndex();
	int LinkAvailableFilePointerContainerWithHashesStruct(Hashes* pHashesStruct, int hashesStructIndex);
	int AssertValidStructLinkage(int resourceID);
	int SIXB44F0(char* fpath, FileAccessType fileAccessType, Hashes* pHashesStruct, int a4);
	unsigned __int64 CalculateStatusDependentValue(Hashes* pHashesStruct, int base);
	int InitializeFilePointerContainerFileHandleID(Hashes* pHashesStruct, int filePointerContainerIndex);

	LARGE_INTEGER SetFilePointer(int resourceID, LARGE_INTEGER distToMove, DWORD moveMethod);
	LARGE_INTEGER SetFilePointer(FilePointerInfo* pFilePointerInfo, LARGE_INTEGER distToMove, DWORD moveMethod);
	LARGE_INTEGER SetFilePointer(FileBufferContainer* pFileBufferContainer, LARGE_INTEGER distToMove, DWORD moveMethod);
	LARGE_INTEGER SetFilePointer(FileHandleContainer* pFileHandleContainer, LARGE_INTEGER distToMove, DWORD moveMethod);

	static constexpr int GetFileBufferContainersCount() { return 20; }

	// resource ID reference values
	static constexpr int FileHandleContainersBase = 0;
	static constexpr int FileBufferContainersBase = 1024;
	static constexpr int FilePointerInfosBase = 2048;
	static constexpr int MaximumValidResourceID = 4096;

	static constexpr int MaxFilePathContainersCount = 100;
	static constexpr int MaxFilePointerInfoCount = 20;

private:
	static inline FileIOManager* _instance = 0;

	static inline int CriticalSectionIndex = -1;
	static inline int CriticalSectionLockCount = 0;
	static inline int FilesReadCounter = 0; // increments when Read is called
	static inline int FilePathContainersCount = 0;
	static inline int bCanFileBeReadNonsequentially = 0; // honestly a shot in the dark, don't really know what this is

	CRITICAL_SECTION* CriticalSectionsArray[14];
	HANDLE FileHandlesArray[32];
	FileHandleContainer FileHandleContainersArray[32];
	FileBufferContainer FileBufferContainersArray[20];
	FileDataContainer FileDataContainersArray[4];
	FilePointerInfo FilePointerInfoArray[MaxFilePointerInfoCount];
	FilePathContainer FilePathContainersArray[MaxFilePathContainersCount];

	FilePathContainer DefaultFilePathContainer{};

	LARGE_INTEGER SomeLargeInteger;

	// wrappers for Windows api
	int RawWrite(int fileHandleIndex, LPVOID lpBuffer, int numberOfBytesToWrite);
	int RawRead(int fileHandleIndex, LPVOID lpBuffer, int numberOfBytesToRead);
	LARGE_INTEGER RawSetFilePointer(int fileHandleIndex, LARGE_INTEGER distToMove, int moveMethod);
	void RawEnterCriticalSection(int criticalSectionIndex);
	void RawLeaveCriticalSection(int criticalSectionIndex);
};

#endif // LEGOINDY_FILEIO_H

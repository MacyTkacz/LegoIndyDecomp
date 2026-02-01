#ifndef LEGOINDY_FILEIO_H
#define LEGOINDY_FILEIO_H

#include <Windows.h>
#include <strings/hash.h>
#include <strings/strings.h>
#include <array>
#include <vector>
#include <string>

// ==================== CONSTANTS ====================

// resource ID reference values
constexpr int RSRCID_FILEHANDLECONTAINERSBASE = 0;
constexpr int RSRCID_FILEBUFFERCONTAINERSBASE = (1<<10);
constexpr int RSRCID_FILEPOINTERINFOSBASE     = (1<<11);
constexpr int RSRCID_MAX					  = (1<<12);

// ===================== STRUCTS =====================

enum FileAccessType { READ, CREATE, MODIFY, OTHER };
enum FileReadErrorCode { HANDLE_CREATION_FAILED_1 = -3, HANDLE_CREATION_FAILED_2, OVERFLOW, NONE };

struct FileHandleContainer;
struct FileDataContainer {
	FileHandleContainer* pFileHandleContainer;
	int lastWriteIndex; // lower number = written to earlier
	char dataBuffer[1024];
};

// for reading raw file data from a file HANDLE, most basic of the resource types
struct FileHandleContainer {
	int fileHandleIndex;
	char pad1[4];
	LARGE_INTEGER filePointerPosition;
	LARGE_INTEGER someLargeInt1;
	LARGE_INTEGER fileEndPosition;
	LARGE_INTEGER someLargeInt2;
	int fileDataLength;
	int someProcessingFlag; // if false, always read from FILE_BEGIN (I think?)
	void* pSomething;
	char pad3[4];
	FileAccessType fileAccessType;
	FileDataContainer* pFileDataContainer;
};

// for reading files compressed in some way
struct FilePointerInfo {
	DATParser* pDATParser;
	DWORD dw1;
	LARGE_INTEGER fileStartPosition;
	LARGE_INTEGER filePointerPosition;
	DWORD fileDataSize;
	DWORD fileDataSizeWhenFileTypeIsNonzero;
	int filePointerContainerIndex;
	DWORD bIsInUse;
	FileType fileType;
	DWORD dw4;
};

// ===================== CLASSES =====================

// for reading wide char file data
class FileBufferContainer {
public:
	char* textBuffer; // 0x0
	char* textBufferEnd; // 0x4
	char* filePointerPosition; // 0x8
	int bSomeBool; // 0xC
	int bIsInUse = 0; // 0x10
};

class DATParser {
public:

	int LinkAvailableFilePointerContainer(int hashesStructIndex);

	int status;
	short someInt16;
	FileAccessType fileAccessType;
	FilePointerContainer filePointerContainersArray[8];

	std::string DATfileName;
	std::string stringsBuffer;
	std::vector<SomeSixteen> SomeSixteenArray;
	std::vector<Hash> hashArray;
	std::vector<int> hashes;

	int someNum1;
	int someNum2;
	int numOfStringHashIndexPairs;
	std::string stringHashIndexPairs;	

};

// singleton class that manages file IO
class FileIOManager {
public:
	FileIOManager();
	static FileIOManager* Instance();

	int TopLevelFileReadingFunction(char* fname, char* textBuffer, int maxDataSize);

	static FileResourceType GetResourceType(int resourceID);

	// reads file data into an available FileDataContainer
	int Read(FileHandleContainer* pFileHandleContainer);
	int CreateFileHandle(LPCSTR fpath, FileAccessType fileAccessType);
	bool CloseFileHandle(int fileHandleIndex);
	void CloseResource(int resourceID);
	int ReadResourceData(int resourceID, char* textBuffer, int numberOfBytesToRead);
	int FilePointerInfoRead(int resourceID, char* textBuffer, int numberOfBytesToRead);

	FilePathContainer* GetFilePathContainerFromPath(char* fpath);
	int FormatAvailableFileBufferContainer(char* buffer, int bufferSize, unsigned int someValue);
	int PopulateFileDataContainer(FileHandleContainer* pFileHandleContainer);
	int GetAvailableFilePointerInfoIndex();
	int GetResourceBufferSize(int resourceID);
	int DoesFileHaveFileHandle(char* fname);

	unsigned __int64 CalculateDataStartPosition(DATParser& DATParser, int base);
	int InitializeFilePointerContainerFileHandleID(DATParser* pDATParser, int filePointerContainerIndex);
	DATParser* InitializeHashesStruct(char* fpath, void** pDATParserAddress, size_t *pSize_out, FileAccessType fileAccessType);
	int SomeLargeFileReadingFunction(DATParser& DATParser, char* fname, FileAccessType fileAccessType);

	int SIXB44F0(char* fpath, FileAccessType fileAccessType, DATParser* pDATParser);
	int SIXB59E0(DATParser& DATParser, char* fname, char* dataBuffer, int maxDataSize);

	int AssertValidStructLinkage(int resourceID);

	LARGE_INTEGER SetFilePointer(int resourceID, LARGE_INTEGER distToMove, DWORD moveMethod);
	LARGE_INTEGER SetFilePointer(FilePointerInfo* pFilePointerInfo, LARGE_INTEGER distToMove, DWORD moveMethod);
	LARGE_INTEGER SetFilePointer(FileBufferContainer* pFileBufferContainer, LARGE_INTEGER distToMove, DWORD moveMethod);
	LARGE_INTEGER SetFilePointer(FileHandleContainer* pFileHandleContainer, LARGE_INTEGER distToMove, DWORD moveMethod);

	int AdvanceCriticalSection();
	static inline int		 GetCriticalSectionIndex() { return CurrentCriticalSectionIndex; }
	static CRITICAL_SECTION* GetCriticalSection(int criticalSectionIndex) { return CriticalSectionsArray[criticalSectionIndex]; }
	static CRITICAL_SECTION* GetCurrentCriticalSection() { return CriticalSectionsArray[CurrentCriticalSectionIndex]; }

	static inline void SetHashesStruct(DATParser* pDATParser) { pSomeDATParser = pDATParser; }

	void LZ2K_AttemptRawRead();
	int  LZ2K_UncompressData(char* compressedDataBuffer, char* uncompressedDataOut, int compressedSizeMinusHeader, int uncompressedSize);

private:
	// singleton instance
	static inline FileIOManager* _instance = 0;

	static inline std::array<CRITICAL_SECTION*, 14> CriticalSectionsArray{ 0 };
	static inline int CriticalSectionLockCount = 0;

	static inline int CurrentCriticalSectionIndex = -1; // is cycled and returned from AdvanceCriticalSection()
	static inline int CriticalSectionIndex_CreateFileHandle = -1;  
	static inline int CriticalSectionIndex_ResourceIndexing = -1;

	static inline int FilesReadCounter = 0; // increments when Read is called
	static inline int FileDataSize = 0;
	static inline int someProcessingFlag = -1; // is set to 0 during the course of some functions' execution
	static inline FileReadErrorCode fileReadErrorCode = FileReadErrorCode::NONE;
	static inline int LatestWriteIndex = 0;
	static inline int NumberOfCharsWritten = 0;
	
	static inline int LZ2KUncompressedFileSize = 0;
	static inline int LZ2KCompressedFileSizeMinusHeader = 0;

	static inline LARGE_INTEGER SomeFileStartPosition{ 0 };
	static inline LARGE_INTEGER SomeLargeInteger{ 0 };

	static inline FilePointerContainer* pSomeFilePointerContainer = 0;
	static inline DATParser* pSomeDATParser = 0;

	static inline std::array<HANDLE, 64> FileHandlesArray{ (HANDLE)-1 };
	static inline std::array<FileHandleContainer, 32> FileHandleContainersArray{0};

	static inline std::array<FileBufferContainer, 16> FileBufferContainersArray{0};
	static inline std::array<FileDataContainer, 4> FileDataContainersArray{0};

	static inline FilePointerInfo* pSomeFilePointerInfo = 0;
	static inline std::array<FilePointerInfo, 16> FilePointerInfoArray{0};

	FilePathContainer DefaultFilePathContainer{};
	static inline int FilePathContainersCount = 0;
	static inline std::array<FilePathContainer, 16> FilePathContainersArray{0};

	static inline char LZ2KCompressedDataBuffer[32768]{0};
	static inline char LZ2KUncompressedDataBuffer[32768]{0};

	// wrappers for Windows api
	int RawWrite(int fileHandleIndex, LPVOID lpBuffer, int numberOfBytesToWrite);
	int RawRead(int fileHandleIndex, LPVOID lpBuffer, int numberOfBytesToRead);
	LARGE_INTEGER RawSetFilePointer(int fileHandleIndex, LARGE_INTEGER distToMove, int moveMethod);
};

// ===================== FUNCTIONS =====================

void RawEnterCriticalSection(int criticalSectionIndex);
void RawLeaveCriticalSection(int criticalSectionIndex);

#endif // LEGOINDY_FILEIO_H

#include "macros.h"
#include "fileio.h"
#include "strings/std.h"
#include "strings/lz2k.h"
#include <utils.h>

// temporary fixes for uninitialized data
FileIOManager::FileIOManager() {

	CriticalSectionsArray[0] = new CRITICAL_SECTION();
	FileDataContainersArray[0].pFileHandleContainer = &FileHandleContainersArray[0];

}

FileIOManager* FileIOManager::Instance() {
	if (!_instance)
		_instance = new FileIOManager();
	return _instance;
}

FileResourceType FileIOManager::GetResourceType(int resourceID) {
	if (resourceID >= RSRCID_MAX || resourceID < 0)
		return FileResourceType::INVALID;
	if (resourceID > RSRCID_FILEPOINTERINFOSBASE)
		return FileResourceType::FILEPOINTERINFO;
	if (resourceID > RSRCID_FILEBUFFERCONTAINERSBASE)
		return FileResourceType::FILEBUFFERCONTAINER;
	return FileResourceType::FILEHANDLECONTAINER;
}

// if within a valid range, increments CurrentCriticalSectionIndex and calls win32 InitializeCriticalSection
// updates CriticalSectionLockCount
int FileIOManager::AdvanceCriticalSection() {
	if (CurrentCriticalSectionIndex > 12)
		return 0;
	CurrentCriticalSectionIndex++;
	InitializeCriticalSection(CriticalSectionsArray[CurrentCriticalSectionIndex]);
	CriticalSectionLockCount = CriticalSectionsArray[CurrentCriticalSectionIndex]->LockCount;
	return CurrentCriticalSectionIndex;
}

int FileIOManager::CreateFileHandle(LPCSTR fpath, FileAccessType fileAccessType) {

	int fileHandleIndex;

	DWORD dwCreationDisposition = 0;
	DWORD dwDesiredAccess = 0;

	if (fileAccessType == FileAccessType::READ) {
		dwDesiredAccess = GENERIC_READ;
		dwCreationDisposition = OPEN_EXISTING; 
		goto ACCESS_TYPE_INITIALIZED;
	}

	if (fileAccessType == FileAccessType::CREATE) {
		dwDesiredAccess = GENERIC_WRITE; 
		dwCreationDisposition = CREATE_ALWAYS; 
		goto ACCESS_TYPE_INITIALIZED;
	}

	if (fileAccessType == FileAccessType::MODIFY) {
		dwDesiredAccess = GENERIC_WRITE; 
		dwCreationDisposition = OPEN_ALWAYS; 
	}

ACCESS_TYPE_INITIALIZED:

	if (CriticalSectionIndex_CreateFileHandle == -1)
		CriticalSectionIndex_CreateFileHandle = AdvanceCriticalSection();
	if (CriticalSectionIndex_CreateFileHandle - 1 <= 11)
		EnterCriticalSection(CriticalSectionsArray[CriticalSectionIndex_CreateFileHandle]);

	int currentFileHandleIndex = 0;
	while (FileHandlesArray[currentFileHandleIndex] != (HANDLE)-1) {
		if (++currentFileHandleIndex >= 64) {
			fileHandleIndex = -1;
			goto FILE_HANDLES_ARRAY_FULL;
		}
	}
	fileHandleIndex = currentFileHandleIndex;

FILE_HANDLES_ARRAY_FULL:

	FileHandlesArray[fileHandleIndex] = CreateFileA(fpath, dwDesiredAccess, 1, 0, dwCreationDisposition, 0, 0);
	if (CriticalSectionIndex_CreateFileHandle - 1 <= 11)
		LeaveCriticalSection(CriticalSectionsArray[CriticalSectionIndex_CreateFileHandle]);
	if (FileHandlesArray[fileHandleIndex] == (HANDLE)-1)
		return -1;

	return fileHandleIndex;

}

bool FileIOManager::CloseFileHandle(int fileHandleIndex) {

	bool wasClosed = CloseHandle(FileHandlesArray[fileHandleIndex]);
	FileHandlesArray[fileHandleIndex] = (HANDLE)-1;
	return !wasClosed;

}

// reverts a resource to its freed state (not in-use)
void FileIOManager::CloseResource(int resourceID) {

	// outside of valid ID range
	if (resourceID >= RSRCID_MAX)
		return;

	if (resourceID >= RSRCID_FILEBUFFERCONTAINERSBASE) {

		// close FileBufferContainer
		if (resourceID < RSRCID_FILEPOINTERINFOSBASE) {
			FileBufferContainersArray[resourceID - RSRCID_FILEBUFFERCONTAINERSBASE].bIsInUse = 0;
			return;
		}

		// close FilePointerInfo
		FilePointerInfo* pFilePointerInfo = &FilePointerInfoArray[resourceID - RSRCID_FILEPOINTERINFOSBASE];
		int filePointerContainerIndex = pFilePointerInfo->filePointerContainerIndex;
		if (filePointerContainerIndex >= 0)
			pFilePointerInfo->pDATParser->filePointerContainersArray[filePointerContainerIndex].fileHandleIndex = -1;
		pFilePointerInfo->bIsInUse = 0;
		return;

	}

	// close FileHandleContainer
	int fileHandleIndex = resourceID - 1;
	CloseFileHandle(fileHandleIndex);

	FileHandleContainer* pFileHandleContainer = &FileHandleContainersArray[fileHandleIndex];
	FileDataContainer* pFileDataContainer = pFileHandleContainer->pFileDataContainer;

	if (pFileDataContainer)
		pFileDataContainer->pFileHandleContainer = 0;

	memset(pFileHandleContainer, 0, sizeof(FileHandleContainer));

}

// calls win32 WriteFile and returns number of bytes written
int FileIOManager::RawWrite(int fileHandleIndex, LPVOID lpBuffer, int numberOfBytesToWrite) {

	HANDLE hFile = FileHandlesArray[fileHandleIndex];
	DWORD numberOfBytesWritten = 0;
	bool success = WriteFile(hFile, lpBuffer, numberOfBytesToWrite, &numberOfBytesWritten, 0);
	return success ? numberOfBytesWritten : 0;

}

// calls win32 ReadFile and returns number of bytes read
int FileIOManager::RawRead(int fileHandleIndex, LPVOID lpBuffer, int numberOfBytesToRead) {

	HANDLE hFile = FileHandlesArray[fileHandleIndex];
	DWORD numberOfBytesRead = 0;
	if (!ReadFile(hFile, lpBuffer, numberOfBytesToRead, &numberOfBytesRead, 0))
		return 0;
	return numberOfBytesRead;

}

int FileIOManager::Read(FileHandleContainer* pFileHandleContainer) {
	
	int fileDataContainerIndex = 0;
	int filesReadCount = FilesReadCounter;
	int oldestWriteIndex = filesReadCount;
	FilesReadCounter++;

	// choose the FileDataContainer written to earliest
	if (FileDataContainersArray[0].lastWriteIndex < filesReadCount) {
		oldestWriteIndex = FileDataContainersArray[0].lastWriteIndex;
	}
	if (FileDataContainersArray[1].lastWriteIndex < oldestWriteIndex) {
		oldestWriteIndex = FileDataContainersArray[1].lastWriteIndex;
		fileDataContainerIndex = 1;
	}
	if (FileDataContainersArray[2].lastWriteIndex < oldestWriteIndex) {
		oldestWriteIndex = FileDataContainersArray[2].lastWriteIndex;
		fileDataContainerIndex = 2;
	}
	if (FileDataContainersArray[3].lastWriteIndex < oldestWriteIndex) {
		fileDataContainerIndex = 3;
	}

	FileDataContainer* pFileDataContainer = &FileDataContainersArray[fileDataContainerIndex];
	if (pFileDataContainer->pFileHandleContainer)
		pFileDataContainer->pFileHandleContainer->pFileDataContainer = 0;
	pFileDataContainer->lastWriteIndex = filesReadCount;
	pFileDataContainer->pFileHandleContainer = pFileHandleContainer;
	
	int fileDataBufferSize = pFileHandleContainer->fileDataLength;
	pFileHandleContainer->pFileDataContainer = pFileDataContainer;

	LARGE_INTEGER distToMove;
	distToMove.QuadPart = (LONGLONG)-fileDataBufferSize;

	if (fileDataBufferSize) {
		RawSetFilePointer(pFileHandleContainer->fileHandleIndex, distToMove, FILE_CURRENT);
		return RawRead(
			pFileHandleContainer->fileHandleIndex,
			pFileDataContainer->dataBuffer,
			pFileHandleContainer->fileDataLength);
	}

	return fileDataBufferSize;

}

// calls win32 SetFilePointerEx on a file HANDLE from FileHandlesArray
LARGE_INTEGER FileIOManager::RawSetFilePointer(int fileHandleIndex, LARGE_INTEGER distToMove, int moveMethod) {

	LARGE_INTEGER newFilePointer;

	if (moveMethod && moveMethod != FILE_CURRENT)
		moveMethod = FILE_END;

	newFilePointer.QuadPart = 0;

	if (SetFilePointerEx(FileHandlesArray[fileHandleIndex], distToMove, &newFilePointer, moveMethod))
		return newFilePointer;

	newFilePointer.QuadPart = -1;
	return newFilePointer;

}

LARGE_INTEGER FileIOManager::SetFilePointer(FilePointerInfo* pFilePointerInfo, LARGE_INTEGER distToMove, DWORD moveMethod) {

	__int64 initialPosition;
	LARGE_INTEGER newPosition;

	FilePointerContainer* pFilePointerContainer = &pFilePointerInfo->pDATParser->filePointerContainersArray[pFilePointerInfo->filePointerContainerIndex];

	switch (moveMethod) {
		case FILE_CURRENT: {
			initialPosition = pFilePointerInfo->filePointerPosition.QuadPart;
			newPosition.QuadPart = distToMove.QuadPart + initialPosition;
			break;
		}
		case FILE_END: {
			newPosition.QuadPart = pFilePointerInfo->fileStartPosition.QuadPart + pFilePointerInfo->fileDataSize - distToMove.QuadPart;
			break;
		}
		default:
		case FILE_BEGIN: {
			initialPosition = pFilePointerInfo->fileStartPosition.QuadPart;
			newPosition.QuadPart = distToMove.QuadPart + initialPosition;
		}
	}

	LARGE_INTEGER filePointerPosition = SetFilePointer(
		pFilePointerContainer->fileHandleID,
		newPosition,
		FILE_BEGIN
	);
	pFilePointerInfo->filePointerPosition.QuadPart = filePointerPosition.QuadPart;
	pFilePointerContainer->filePointerPosition.QuadPart = filePointerPosition.QuadPart;
	return filePointerPosition;

}

LARGE_INTEGER FileIOManager::SetFilePointer(FileBufferContainer* pFileBufferContainer, LARGE_INTEGER distToMove, DWORD moveMethod) {

	switch(moveMethod) {
		case FILE_CURRENT: {
			pFileBufferContainer->filePointerPosition += distToMove.QuadPart;
			return ToLargeInt(pFileBufferContainer->filePointerPosition - pFileBufferContainer->textBuffer);
		}
		case FILE_END: {
			char* newFilePointer = &pFileBufferContainer->textBufferEnd[-distToMove.QuadPart];
			pFileBufferContainer->filePointerPosition = newFilePointer;
			return ToLargeInt(newFilePointer - pFileBufferContainer->textBuffer);
		}
		case FILE_BEGIN:
		default: {
			pFileBufferContainer->filePointerPosition = &pFileBufferContainer->textBuffer[distToMove.QuadPart];
			return distToMove;
		}
	}

}

LARGE_INTEGER FileIOManager::SetFilePointer(FileHandleContainer* pFileHandleContainer, LARGE_INTEGER distToMove, DWORD moveMethod) {

	if (!pFileHandleContainer->someProcessingFlag)
		return RawSetFilePointer(pFileHandleContainer->fileHandleIndex, distToMove, moveMethod);

	switch (moveMethod) {
		case FILE_CURRENT: {
			pFileHandleContainer->filePointerPosition.QuadPart += distToMove.QuadPart;
			return pFileHandleContainer->filePointerPosition;
		}
		case FILE_END: {
			pFileHandleContainer->filePointerPosition.QuadPart = pFileHandleContainer->fileEndPosition.QuadPart - distToMove.QuadPart;
			return pFileHandleContainer->filePointerPosition;
		}
		case FILE_BEGIN:
		default: {
			pFileHandleContainer->filePointerPosition.QuadPart = distToMove.QuadPart;
			return pFileHandleContainer->filePointerPosition;
		}
	}

}

LARGE_INTEGER FileIOManager::SetFilePointer(int resourceID, LARGE_INTEGER distToMove, DWORD moveMethod) {

	if (resourceID >= RSRCID_MAX)
		return LARGE_INTEGER{ 0 };

	if (resourceID < RSRCID_FILEBUFFERCONTAINERSBASE)
		return SetFilePointer(&FileHandleContainersArray[resourceID - RSRCID_FILEHANDLECONTAINERSBASE], distToMove, moveMethod);

	if (resourceID < RSRCID_FILEPOINTERINFOSBASE)
		return SetFilePointer(&FileBufferContainersArray[resourceID - RSRCID_FILEBUFFERCONTAINERSBASE], distToMove, moveMethod);

	return SetFilePointer(&FilePointerInfoArray[resourceID - RSRCID_FILEPOINTERINFOSBASE], distToMove, moveMethod);

}

// attempts to find a FileBufferContainer from FileBufferContainersArray which is not in use
// if found, marks it as in-use and sets it default values
int FileIOManager::FormatAvailableFileBufferContainer(char* buffer, int bufferSize, unsigned int bSomeBool) {

	if (bufferSize <= 0 || bSomeBool > 1)
		return 0;

	int i;
	for (i = 0; FileBufferContainersArray[i].bIsInUse; i++) {
		if (i == FileBufferContainersArray.size()) return 0;
	}

	FileBufferContainer* pFileBufferContainer = &FileBufferContainersArray[i];

	pFileBufferContainer->textBufferEnd = &buffer[bufferSize - 1];
	pFileBufferContainer->bSomeBool = bSomeBool;
	pFileBufferContainer->textBuffer = buffer;
	pFileBufferContainer->filePointerPosition = buffer;
	pFileBufferContainer->bIsInUse = 1;

	return i + RSRCID_FILEBUFFERCONTAINERSBASE;

}

// calls win32 EnterCriticalSection on an object in the CriticalSectionsArray
void RawEnterCriticalSection(int criticalSectionIndex) {

	FileIOManager* fiom = FileIOManager::Instance();
	if (!fiom)
		return;

	CRITICAL_SECTION* criticalSection = fiom->GetCriticalSection(criticalSectionIndex);

	if (criticalSectionIndex - 1 <= 11)
		EnterCriticalSection( criticalSection );

}

// calls win32 LeaveCriticalSection on an object in the CriticalSectionsArray
void RawLeaveCriticalSection(int criticalSectionIndex) {

	FileIOManager* fiom = FileIOManager::Instance();
	if (!fiom)
		return;

	CRITICAL_SECTION* criticalSection = fiom->GetCriticalSection(criticalSectionIndex);

	if (criticalSectionIndex - 1 <= 11)
		LeaveCriticalSection( criticalSection );

}

// attempts to match an absolute path with that of a FilePathContainer from FilePathContainersArray
FilePathContainer* FileIOManager::GetFilePathContainerFromPath(char* fpath) {

	int currentCharIndex = 0;
	while (fpath[currentCharIndex] != ':') {
		if (++currentCharIndex >= 8)
			return 0;
	}

	for (int i; i < FilePathContainersArray.size(); i++) {
		FilePathContainer* pFilePathContainer = &FilePathContainersArray[i];
		if (!_strncmp(fpath, pFilePathContainer->path, pFilePathContainer->pathLength ) )
			return pFilePathContainer;
	}

	return 0;

}

// this function can either return 0 or error out
// my best guess is that this is an assertion for a proper linkage between a FilePointerInfo and a FilePointerContainer?
// note: in the codebase, this is only ever run after a check for a FileAccessType of 3
int FileIOManager::AssertValidStructLinkage(int resourceID) {

	int i = resourceID;
	while (i < RSRCID_FILEBUFFERCONTAINERSBASE) {
		FilePointerInfo* pFilePointerInfo = &FilePointerInfoArray[i - RSRCID_FILEPOINTERINFOSBASE];
		int filePointerContainerIndex = pFilePointerInfo->filePointerContainerIndex;
		i = pFilePointerInfo->pDATParser->filePointerContainersArray[filePointerContainerIndex].fileHandleID;
		if (i < RSRCID_FILEPOINTERINFOSBASE)
			break;
	}
	return 0;

}

// TODO: find out wtf is happening here and rename
int FileIOManager::SIXB44F0(char* fpath, FileAccessType fileAccessType, DATParser* pDATParser) {

	int resourceID;
	LARGE_INTEGER newFilePointerPosition;

	FilePathContainer* pFilePathContainer = GetFilePathContainerFromPath(fpath);
	if (!pFilePathContainer ) {

		if ( pDATParser && fileAccessType == FileAccessType::READ ) {
			int resourceID = SomeLargeFileReadingFunction(*pDATParser, fpath, fileAccessType);
			if (resourceID)
				return resourceID;
		}

		pFilePathContainer = &DefaultFilePathContainer;

	}

	char joinedFpath[256];
	pFilePathContainer->pathJoiningFunction(pFilePathContainer, joinedFpath, fpath, 256);

	if (pFilePathContainer->filePathInfo.status == 1)
		return RSRCID_MAX;

	int fileHandleIndex = CreateFileHandle(joinedFpath, fileAccessType);
	if (fileHandleIndex < 0)
		return 0;

	FileHandleContainer* pFileHandleContainer = &FileHandleContainersArray[fileHandleIndex];
	memset(pFileHandleContainer, 0, sizeof(FileHandleContainer));
	pFileHandleContainer->fileHandleIndex = fileHandleIndex;
	pFileHandleContainer->fileAccessType = fileAccessType;

	if (fileAccessType == CREATE) {
		pFileHandleContainer->fileEndPosition.QuadPart = 0;
	}
	else {
		if (fileHandleIndex > 16)
			goto LABEL_25;
		while (true) {
			newFilePointerPosition = SetFilePointer(fileHandleIndex + 1, LARGE_INTEGER{ 0 }, FILE_END);
			if (newFilePointerPosition.HighPart >= 0)
				break;
		}
		FileHandleContainersArray[fileHandleIndex].fileEndPosition = newFilePointerPosition;
		if (fileAccessType == ( FileAccessType::MODIFY | FileAccessType::CREATE ))
			while (AssertValidStructLinkage(fileHandleIndex+1));
		while (true) {
			newFilePointerPosition = SetFilePointer(fileHandleIndex + 1, LARGE_INTEGER{ 0 }, FILE_BEGIN);
			if (newFilePointerPosition.HighPart >= 0)
				break;
		}
		if (fileAccessType == ( FileAccessType::MODIFY | FileAccessType::CREATE ))
			while (AssertValidStructLinkage(fileHandleIndex+1));
		else {
LABEL_25:
			if (fileAccessType == FileAccessType::READ)
				FileHandleContainersArray[fileHandleIndex].someProcessingFlag = someProcessingFlag;
		}
	}
	FileHandleContainersArray[fileHandleIndex].pSomething = 0;
	return fileHandleIndex + 1;

}

// mark a FilePointerInfo from FilePointerInfoArray as in-use and return its index
int FileIOManager::GetAvailableFilePointerInfoIndex() {

	int availableIndex = -1;

	RawEnterCriticalSection(CriticalSectionIndex_ResourceIndexing);
	for (int i = 0; i < FilePointerInfoArray.size(); i++) {
		FilePointerInfo* pFilePointerInfo = &FilePointerInfoArray[i];
		if (!pFilePointerInfo->bIsInUse) {
			availableIndex = i;
			break;
		}
	}
	if (availableIndex != -1)
		FilePointerInfoArray[availableIndex].bIsInUse = 1;
	RawLeaveCriticalSection(CriticalSectionIndex_ResourceIndexing);
	return availableIndex;

}

// returns chunkNumber*256 if DATParser.status <= -2
unsigned __int64 FileIOManager::CalculateDataStartPosition(DATParser& DATParser, int chunkNumber) {

	bool flag = DATParser.status <= -2;
	return SomeLargeInteger.QuadPart + (static_cast<__int64>(chunkNumber) << (flag ? 8 : 0));

}

int FileIOManager::InitializeFilePointerContainerFileHandleID(DATParser* pDATParser, int filePointerContainerIndex) {

	// return FilePointerContainer's fileHandleID, if any
	FilePointerContainer* pFilePointerContainer = &pDATParser->filePointerContainersArray[filePointerContainerIndex];
	if (pFilePointerContainer->fileHandleID)
		return pFilePointerContainer->fileHandleID;

	// give FilePointerContainer a new fileHandleID and reset its FilePointerPosition to 0
	int fileHandleID = SIXB44F0(const_cast<char*>(pDATParser->DATfileName.c_str()), (FileAccessType)pDATParser->fileAccessType, 0);
	pFilePointerContainer->fileHandleID = fileHandleID;
	pFilePointerContainer->filePointerPosition.QuadPart = 0;
	return fileHandleID;

}

// initializes and returns a FilePointerInfo
int FileIOManager::SomeLargeFileReadingFunction(DATParser& DATParser, char* fname, FileAccessType fileAccessType) {

	// return if not READ
	if (fileAccessType)
		return 0;

	int someStructIndex = GetFormattedHashIndex(&DATParser, fname);
	if (someStructIndex < 0 || !DATParser.SomeSixteenArray[someStructIndex].int2)
		return 0;

	int filePointerInfoIndex = GetAvailableFilePointerInfoIndex();
	if (filePointerInfoIndex == -1)
		return 0;

	int filePointerContainerIndex = DATParser.LinkAvailableFilePointerContainer(filePointerInfoIndex);
	if (filePointerContainerIndex == -1) {
		FilePointerInfoArray[filePointerInfoIndex].bIsInUse = 0;
		return 0;
	}

	FilePointerInfo* pFilePointerInfo = &FilePointerInfoArray[filePointerInfoIndex];
	FilePointerContainer* pFilePointerContainer = &DATParser.filePointerContainersArray[filePointerContainerIndex];

	InitializeFilePointerContainerFileHandleID(&DATParser, filePointerContainerIndex);
	pFilePointerInfo->pDATParser = &DATParser;

	LARGE_INTEGER fileStart = ToLargeInt( CalculateDataStartPosition(DATParser, DATParser.SomeSixteenArray[someStructIndex].chunkNumber) );
	
	pFilePointerInfo->fileStartPosition = fileStart;
	pFilePointerInfo->filePointerPosition = fileStart;
	pFilePointerInfo->fileDataSize = DATParser.SomeSixteenArray[someStructIndex].int2;
	pFilePointerInfo->fileDataSizeWhenFileTypeIsNonzero = DATParser.SomeSixteenArray[someStructIndex].int3;
	pFilePointerInfo->fileType = static_cast<FileType>( DATParser.SomeSixteenArray[someStructIndex].int4 );
	pFilePointerInfo->filePointerContainerIndex = filePointerContainerIndex;
	pFilePointerContainer->filePointerPosition = fileStart;

	LARGE_INTEGER distToMove;

	while (true) {
		distToMove = pFilePointerInfo->fileStartPosition;
		if ((SetFilePointer(pFilePointerContainer->fileHandleID, distToMove, FILE_BEGIN).HighPart & GENERIC_READ) == 0LL)
			break;
	}

	if (pFilePointerInfo->fileType == FileType::LZ2K) {
		pSomeFilePointerInfo = pFilePointerInfo;
		pSomeFilePointerContainer = pFilePointerContainer;
		SomeFileStartPosition = pFilePointerInfo->fileStartPosition;
		FileDataSize = 0;
	}
	return filePointerInfoIndex + RSRCID_FILEPOINTERINFOSBASE;

}

// attempts to link FileHandleContainer to a global FileDataContainer and read in file data
int FileIOManager::PopulateFileDataContainer(FileHandleContainer* pFileHandleContainer) {

	int latestWriteIndex = LatestWriteIndex;
	int oldestWriteIndex = latestWriteIndex;
	int fileDataContainerIndex = 0;

	// chooses the FileDataContainer written to earliest
	if (FileDataContainersArray[0].lastWriteIndex < latestWriteIndex) {
		oldestWriteIndex = FileDataContainersArray[0].lastWriteIndex;
		goto CHOOSE_FILEDATACONTAINER;
	}
	if (FileDataContainersArray[1].lastWriteIndex < latestWriteIndex) {
		oldestWriteIndex = FileDataContainersArray[1].lastWriteIndex;
		fileDataContainerIndex = 1;
		goto CHOOSE_FILEDATACONTAINER;
	}
	if (FileDataContainersArray[2].lastWriteIndex < latestWriteIndex) {
		oldestWriteIndex = FileDataContainersArray[2].lastWriteIndex;
		fileDataContainerIndex = 2;
		goto CHOOSE_FILEDATACONTAINER;
	}
	if (FileDataContainersArray[3].lastWriteIndex < latestWriteIndex) {
		fileDataContainerIndex = 3;
	}

CHOOSE_FILEDATACONTAINER:

	FileDataContainer* pFileDataContainer = &FileDataContainersArray[fileDataContainerIndex];
	if (pFileDataContainer->pFileHandleContainer)
		pFileDataContainer->pFileHandleContainer = 0;
	pFileDataContainer->lastWriteIndex = latestWriteIndex;
	pFileDataContainer->pFileHandleContainer = pFileHandleContainer;

	int fileDataBufferSize = pFileHandleContainer->fileDataLength;
	LatestWriteIndex = latestWriteIndex + 1;
	pFileHandleContainer->pFileDataContainer = pFileDataContainer;

	if (!fileDataBufferSize)
		return 0;

	RawSetFilePointer(pFileHandleContainer->fileHandleIndex, ToLargeInt(-fileDataBufferSize), FILE_CURRENT);
	return RawRead(
		pFileHandleContainer->fileHandleIndex,
		pFileDataContainer->dataBuffer,
		pFileHandleContainer->fileDataLength);

}

int FileIOManager::GetResourceBufferSize(int resourceID) {

	if (resourceID >= RSRCID_MAX)
		return 0;

	if (resourceID >= RSRCID_FILEPOINTERINFOSBASE) {
		FilePointerInfo* pFilePointerInfo = &FilePointerInfoArray[resourceID - RSRCID_FILEPOINTERINFOSBASE];
		if (pFilePointerInfo->fileType)
			return pFilePointerInfo->fileDataSizeWhenFileTypeIsNonzero;
		return pFilePointerInfo->fileDataSize;
	}

	if (resourceID < RSRCID_FILEBUFFERCONTAINERSBASE)
		// return end position of FileHandleContainer buffer (relative)
		return FileHandleContainersArray[resourceID-1].fileEndPosition.LowPart;

	FileBufferContainer* pFileBufferContainer = &FileBufferContainersArray[resourceID-RSRCID_FILEBUFFERCONTAINERSBASE];
	return pFileBufferContainer->textBufferEnd - pFileBufferContainer->textBuffer;	

}

int FileIOManager::ReadResourceData(int resourceID, char* textBuffer, int numberOfBytesToRead) {

	if (resourceID >= RSRCID_MAX)	
		return 0;

	if (resourceID >= RSRCID_FILEBUFFERCONTAINERSBASE) {

		if (resourceID >= RSRCID_FILEPOINTERINFOSBASE)
			return FilePointerInfoRead(resourceID, textBuffer, numberOfBytesToRead);

		FileBufferContainer* pFileBufferContainer = &FileBufferContainersArray[resourceID - RSRCID_FILEBUFFERCONTAINERSBASE];
		char* filePointerPosition = pFileBufferContainer->filePointerPosition;

		int distToEndOfBuffer = pFileBufferContainer->textBufferEnd - filePointerPosition + 1;
		if ( distToEndOfBuffer < numberOfBytesToRead )
			numberOfBytesToRead = distToEndOfBuffer;

		if (numberOfBytesToRead) {
			memcpy(textBuffer, filePointerPosition, numberOfBytesToRead);
			pFileBufferContainer->filePointerPosition += numberOfBytesToRead;
		}

		return numberOfBytesToRead;

	}
	
	FileHandleContainer* pFileHandleContainer = &FileHandleContainersArray[resourceID - 1];
	if (!pFileHandleContainer->someProcessingFlag)
		return RawRead(resourceID-1, textBuffer, numberOfBytesToRead);

	if (!pFileHandleContainer->pFileDataContainer)
		PopulateFileDataContainer(pFileHandleContainer);

	FileDataContainer* pFileDataContainer = pFileHandleContainer->pFileDataContainer;

	char* textBufferOffset = textBuffer;

	int totalBytesRead = 0;
	if (numberOfBytesToRead <= 0)
		return 0;

	int readSize = numberOfBytesToRead;
	do {

		if (pFileHandleContainer->filePointerPosition.QuadPart >= pFileHandleContainer->fileEndPosition.QuadPart)
			break;

		if (pFileHandleContainer->filePointerPosition.QuadPart < pFileHandleContainer->someLargeInt2.QuadPart ||
		pFileHandleContainer->filePointerPosition.QuadPart >= pFileHandleContainer->someLargeInt2.QuadPart + pFileHandleContainer->fileDataLength) {

			if (pFileHandleContainer->filePointerPosition.QuadPart != pFileHandleContainer->someLargeInt1.QuadPart) {

				RawSetFilePointer(
					pFileHandleContainer->fileHandleIndex,
					pFileHandleContainer->filePointerPosition,
					FILE_BEGIN);

				pFileHandleContainer->someLargeInt1 = pFileHandleContainer->filePointerPosition;

			}				

			int bytesRead = RawRead(
				pFileHandleContainer->fileHandleIndex,
				pFileDataContainer->dataBuffer,
				1024);

			pFileHandleContainer->fileDataLength = bytesRead;
			pFileHandleContainer->someLargeInt2 = pFileHandleContainer->someLargeInt1; 
			pFileHandleContainer->someLargeInt1.QuadPart += bytesRead;

		}

		readSize = pFileHandleContainer->someLargeInt2.LowPart - pFileHandleContainer->filePointerPosition.LowPart + pFileHandleContainer->fileDataLength;
		if (readSize > numberOfBytesToRead)
			readSize = numberOfBytesToRead;

		if (readSize) {
			int dataOffset = pFileHandleContainer->filePointerPosition.LowPart - pFileHandleContainer->someLargeInt2.LowPart;
			memcpy(textBufferOffset, &pFileDataContainer->dataBuffer[dataOffset], readSize);
			textBufferOffset += readSize;
			totalBytesRead += readSize;
			pFileHandleContainer->filePointerPosition.QuadPart += readSize;
			numberOfBytesToRead -= readSize;
		}

	}
	while ( numberOfBytesToRead - readSize > 0 );
	return totalBytesRead;	

}

int FileIOManager::FilePointerInfoRead(int resourceID, char* dataBuffer, int numberOfBytesToRead) {

	FilePointerInfo* fpi = &FilePointerInfoArray[resourceID - RSRCID_FILEPOINTERINFOSBASE];
	FilePointerContainer* fpc;
	{
		int index = fpi->filePointerContainerIndex;
		fpc = &fpi->pDATParser->filePointerContainersArray[index];
	}

	if (fpi->fileType == FileType::LZ2K) {

		if (!numberOfBytesToRead)
			return 0;

		int remainingBytes = numberOfBytesToRead;
		int bytesRead = 0;
		char* currentBufferOffset = dataBuffer;
		int uncompressedFileSize = FileDataSize;

		do {

			if (!uncompressedFileSize) {

				LZ2K_AttemptRawRead();
				if (LZ2KCompressedFileSizeMinusHeader == LZ2KUncompressedFileSize)
					memcpy(LZ2KUncompressedDataBuffer,LZ2KCompressedDataBuffer,LZ2KCompressedFileSizeMinusHeader);
				else
					LZ2K_UncompressData(
						LZ2KCompressedDataBuffer,
						LZ2KUncompressedDataBuffer,
						LZ2KCompressedFileSizeMinusHeader,
						LZ2KUncompressedFileSize
					);

				uncompressedFileSize = LZ2KUncompressedFileSize;
				NumberOfCharsWritten = 0;
				
			}

			int numOfBytesReading = remainingBytes;
			if (remainingBytes >= uncompressedFileSize)
				numOfBytesReading = uncompressedFileSize;

			memcpy(currentBufferOffset,&LZ2KUncompressedDataBuffer[NumberOfCharsWritten],numOfBytesReading);
			NumberOfCharsWritten += numOfBytesReading;
			bytesRead += numOfBytesReading;
			uncompressedFileSize -= numOfBytesReading;
			remainingBytes -= numOfBytesReading;
			currentBufferOffset += numOfBytesReading;
			FileDataSize = uncompressedFileSize;

		}
		while ( remainingBytes );

		return bytesRead;

	}

	if (fpi->filePointerPosition.QuadPart != fpc->filePointerPosition.QuadPart) {

		SetFilePointer(fpc->fileHandleID,fpi->filePointerPosition,FILE_BEGIN);
		fpc->filePointerPosition = fpi->filePointerPosition;

	}

	int remainingBytes = numberOfBytesToRead;

	__int64 fpiOffsetFromEndOfData = fpi->fileStartPosition.QuadPart + fpi->fileDataSize - fpi->filePointerPosition.QuadPart;
	if (fpiOffsetFromEndOfData < 0)
		fpiOffsetFromEndOfData = 0;

	if (numberOfBytesToRead >= fpiOffsetFromEndOfData)	
		remainingBytes = fpiOffsetFromEndOfData;

	if (!remainingBytes)
		return 0;

	int bytesRead = ReadResourceData(fpc->fileHandleID, dataBuffer, remainingBytes);
	if (bytesRead < 0)
		goto RETURN;	

	fpi->filePointerPosition.QuadPart += bytesRead;
	fpc->filePointerPosition = fpi->filePointerPosition;

RETURN:
	return bytesRead;

}

int FileIOManager::DoesFileHaveFileHandle(char* fname) {

	if (!fname || !*fname)
		return 0;

	int someProcessingFlag = FileIOManager::someProcessingFlag;
	FileIOManager::someProcessingFlag = 0;

	DATParser* pDATParser = pSomeDATParser;
	int stringHashIndex = GetFormattedHashIndex(pDATParser, fname);

	if (!pDATParser || stringHashIndex < 0) {
		int resourceID = SIXB44F0(fname, FileAccessType::READ, pDATParser);
		if (resourceID)
			CloseResource(resourceID);
		FileIOManager::someProcessingFlag = someProcessingFlag;	
		return resourceID != 0;
	}

	FileIOManager::someProcessingFlag = someProcessingFlag;
	return pDATParser->SomeSixteenArray[stringHashIndex].int3;

}

// reads data and updates filePointerPosition for FilePointerInfo and FilePointerContainer
int FileIOManager::SIXB59E0(DATParser& DATParser, char* fname, char* dataBuffer, int maxDataSize) {

	int dataSize = 0;

	int resourceID = SomeLargeFileReadingFunction(DATParser, fname, FileAccessType::READ);
	if (!resourceID)
		return 0;

	if (DATParser.fileAccessType == FileAccessType::OTHER)
		while ( AssertValidStructLinkage(resourceID) );


	FilePointerInfo* pFilePointerInfo = &FilePointerInfoArray[resourceID - RSRCID_FILEPOINTERINFOSBASE];
	if (pFilePointerInfo->fileType)
		dataSize = pFilePointerInfo->fileDataSizeWhenFileTypeIsNonzero;
	else
		dataSize = pFilePointerInfo->fileDataSize;

	if (!dataSize) {
		fileReadErrorCode = dataSize > maxDataSize ? FileReadErrorCode::OVERFLOW : FileReadErrorCode::NONE;
		CloseResource(resourceID);
		return 0;
	}

	int filePointerContainerIndex = pFilePointerInfo->filePointerContainerIndex;
	FilePointerContainer* pFilePointerContainer = &pFilePointerInfo->pDATParser->filePointerContainersArray[filePointerContainerIndex];
	resourceID = pFilePointerContainer->fileHandleID;
	__int64 fileStartPosition = pFilePointerInfo->fileStartPosition.QuadPart;

	while ( pFilePointerInfo->filePointerPosition.QuadPart < 0 || FilePointerInfoRead(resourceID,dataBuffer,dataSize) < 0 ) {

		// there's definitely a better way to do this without all these repated conditionals
		// but we're moving on
		if (resourceID < RSRCID_MAX) {

			if (resourceID < RSRCID_FILEBUFFERCONTAINERSBASE) {
				int index = resourceID - 1;
				FileHandleContainer* pFileHandleContainer = &FileHandleContainersArray[index];
				if (pFileHandleContainer->someProcessingFlag) {
					pFileHandleContainer->filePointerPosition.QuadPart = fileStartPosition;
				}
				else {
					// i don't really get it but i think this is correct?
					pFileHandleContainer->filePointerPosition = RawSetFilePointer(index,ToLargeInt(fileStartPosition),FILE_BEGIN);
				}
			}
			else if ( resourceID < RSRCID_FILEPOINTERINFOSBASE ) {
				char* buff = FileBufferContainersArray[resourceID - 1].textBuffer;
				FileBufferContainer* pFileBufferContainer = &FileBufferContainersArray[resourceID - 1];
				pFileBufferContainer->filePointerPosition = fileStartPosition + buff; 
			}
			else {
				fileStartPosition = SetFilePointer(pFilePointerInfo,ToLargeInt(fileStartPosition),FILE_BEGIN).QuadPart;
			}

		}
		else {
			fileStartPosition = 0;
		}

		pFilePointerInfo->filePointerPosition.QuadPart = fileStartPosition;
		pFilePointerContainer->filePointerPosition.QuadPart = fileStartPosition;

	}

	if (DATParser.fileAccessType == FileAccessType::OTHER)
		while (AssertValidStructLinkage(resourceID));

	CloseResource(resourceID);
	return dataSize;

}

int FileIOManager::TopLevelFileReadingFunction(char* fname, char* textBuffer, int maxDataSize) {

	int result = 0;
	fileReadErrorCode = FileReadErrorCode::NONE;

	if (pSomeDATParser) {
		if ( result = SIXB59E0(*pSomeDATParser,fname,textBuffer,maxDataSize), result )
			return result;
	}

	fileReadErrorCode = FileReadErrorCode::HANDLE_CREATION_FAILED_2;
	return result;

	// there is a lot more to this function, but in testing it never goes past here
	// so I'll omit it for now

}

DATParser* FileIOManager::InitializeHashesStruct(char* fpath, void** pDATParserAddress, size_t* pSize_out, FileAccessType fileAccessType) {

	return 0; // CONTINUE HERE

}

void FileIOManager::LZ2K_AttemptRawRead() {

	char fileHeaderBuffer[12];
	ReadResourceData(pSomeFilePointerContainer->fileHandleID, fileHeaderBuffer, 12);

	LZ2KUncompressedFileSize = LZ2K_DecodeUncompressedFileSize(fileHeaderBuffer);
	LZ2KCompressedFileSizeMinusHeader = LZ2K_DecodeCompressedFileSize(fileHeaderBuffer) - 12;

	ReadResourceData(pSomeFilePointerContainer->fileHandleID, LZ2KCompressedDataBuffer, LZ2KCompressedFileSizeMinusHeader);
	pSomeFilePointerInfo->filePointerPosition.QuadPart += LZ2KCompressedFileSizeMinusHeader + 12;

	pSomeFilePointerContainer->filePointerPosition = pSomeFilePointerInfo->filePointerPosition;

}

int FileIOManager::LZ2K_UncompressData(char* compressedDataBuffer, char* uncompressedDataOut, int compressedSizeMinusHeader, int uncompressedSize) {
	return 0;
}

// attempts to set the fileHandleIndex of one of 8 available FilePointerContainers from FilePointerContainersArray
int DATParser::LinkAvailableFilePointerContainer(int hashesStructIndex) {

	FileIOManager* fiom = FileIOManager::Instance();
	if (!fiom)
		return -1;

	RawEnterCriticalSection( fiom->GetCriticalSectionIndex() );

	int availableFilePointerContainerIndex = 0;
	for (int i = 0; this->filePointerContainersArray[i].fileHandleIndex != -1; i++) {
		if (++availableFilePointerContainerIndex >= 8) {
			availableFilePointerContainerIndex = -1;
			goto EXIT;
		}
	}
	this->filePointerContainersArray[availableFilePointerContainerIndex].fileHandleIndex = hashesStructIndex;

EXIT:
	RawLeaveCriticalSection( fiom->GetCriticalSectionIndex() );
	return availableFilePointerContainerIndex;

}
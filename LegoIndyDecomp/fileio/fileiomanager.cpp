#include "fileio.h"
#include "strings/std.h"
#include <utils.h>

// temporary fixes for uninitialized data
FileIOManager::FileIOManager() {
	CriticalSectionsArray[0] = new CRITICAL_SECTION();
	memset(FileHandlesArray, (__int32)-1, 32);
	FileDataContainersArray[0].pFileHandleContainer = &FileHandleContainersArray[0];
}

FileIOManager* FileIOManager::Instance() {
	if (!_instance)
		_instance = new FileIOManager();
	return _instance;
}

int FileIOManager::AdvanceCriticalSection() {
	if (CriticalSectionIndex > 12)
		return 0;
	CriticalSectionIndex++;
	InitializeCriticalSection(CriticalSectionsArray[CriticalSectionIndex]);
	CriticalSectionLockCount = CriticalSectionsArray[CriticalSectionIndex]->LockCount;
	return CriticalSectionIndex;
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

	if (CriticalSectionIndex == -1)
		CriticalSectionIndex = AdvanceCriticalSection();
	if (CriticalSectionIndex - 1 <= 11)
		EnterCriticalSection(CriticalSectionsArray[CriticalSectionIndex]);

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
	if (CriticalSectionIndex - 1 <= 11)
		LeaveCriticalSection(CriticalSectionsArray[CriticalSectionIndex]);
	if (FileHandlesArray[fileHandleIndex] == (HANDLE)-1)
		return -1;

	return fileHandleIndex;

}

bool FileIOManager::CloseFileHandle(int fileHandleIndex) {

	bool wasClosed = CloseHandle(FileHandlesArray[fileHandleIndex]);
	FileHandlesArray[fileHandleIndex] = (HANDLE)-1;
	return !wasClosed;

}

void FileIOManager::CloseResource(int resourceID) {

	// outside of valid ID range
	if (resourceID >= MaximumValidResourceID)
		return;

	if (resourceID >= FileBufferContainersBase) {

		// close FilePointerInfo
		if (resourceID >= FilePointerInfosBase)
			return; // NOT YET IMPLEMENTED

		// close FileBufferContainer
		*(&FileBufferContainersArray[resourceID-FileBufferContainersBase].bIsInUse) = 0;
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

int FileIOManager::RawWrite(int fileHandleIndex, LPVOID lpBuffer, int numberOfBytesToWrite) {

	HANDLE hFile = FileHandlesArray[fileHandleIndex];
	DWORD numberOfBytesWritten = 0;
	bool success = WriteFile(hFile, lpBuffer, numberOfBytesToWrite, &numberOfBytesWritten, 0);
	return success ? numberOfBytesWritten : 0;

}

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

	FilePointerContainer* pFilePointerContainer = &pFilePointerInfo->pHashesStruct->filePointerContainersArray[pFilePointerInfo->filePointerContainerIndex];

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

	if (!pFileHandleContainer->bCanFileBeReadNonsequentially)
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

	if (resourceID >= MaximumValidResourceID)
		return LARGE_INTEGER{ 0 };

	if (resourceID < FileBufferContainersBase)
		return SetFilePointer(&FileHandleContainersArray[resourceID - FileHandleContainersBase], distToMove, moveMethod);

	if (resourceID < FilePointerInfosBase)
		return SetFilePointer(&FileBufferContainersArray[resourceID - FileBufferContainersBase], distToMove, moveMethod);

	SetFilePointer(&FilePointerInfoArray[resourceID - FilePointerInfosBase], distToMove, moveMethod);

}

int FileIOManager::FormatAvailableFileBufferContainer(char* buffer, int bufferSize, unsigned int bSomeBool) {

	if (bufferSize <= 0 || bSomeBool > 1)
		return 0;

	FileBufferContainer* fileBufferContainersArray = Instance()->FileBufferContainersArray;
	
	int i;
	for (i = 0; fileBufferContainersArray[i].bIsInUse; i++) {
		if (i == GetFileBufferContainersCount()) return 0;
	}

	FileBufferContainer* pFileBufferContainer = &fileBufferContainersArray[i];

	pFileBufferContainer->textBufferEnd = &buffer[bufferSize - 1];
	pFileBufferContainer->bSomeBool = bSomeBool;
	pFileBufferContainer->textBuffer = buffer;
	pFileBufferContainer->filePointerPosition = buffer;
	pFileBufferContainer->bIsInUse = 1;

	return i + FileBufferContainersBase;

}

void FileIOManager::RawEnterCriticalSection(int criticalSectionIndex) {
	if (CriticalSectionIndex - 1 <= 11)
		EnterCriticalSection(CriticalSectionsArray[CriticalSectionIndex]);
}

void FileIOManager::RawLeaveCriticalSection(int criticalSectionIndex) {
	if (CriticalSectionIndex - 1 <= 11)
		LeaveCriticalSection(CriticalSectionsArray[CriticalSectionIndex]);
}

FilePathContainer* FileIOManager::GetFilePathContainerFromPath(char* fpath) {

	int currentCharIndex = 0;
	while (fpath[currentCharIndex] != ':') {
		if (++currentCharIndex >= 8)
			return 0;
	}

	for (FilePathContainer filePathContainer : FilePathContainersArray) {
		if (!_strncmp(fpath, filePathContainer.absolutePath, filePathContainer.pathLength ) )
			return &filePathContainer;
	}

	return 0;

}

// this function can either return 0 or error out
// my best guess is that this is an assertion for a proper linkage between a FilePointerInfo and a FilePointerContainer?
// note: in the codebase, this is only ever run after a check for a FileAccessType of 3
int FileIOManager::AssertValidStructLinkage(int resourceID) {

	int i = resourceID;
	while (i < FileBufferContainersBase) {
		FilePointerInfo* pFilePointerInfo = &FilePointerInfoArray[i - FilePointerInfosBase];
		int filePointerContainerIndex = pFilePointerInfo->filePointerContainerIndex;
		i = pFilePointerInfo->pHashesStruct->filePointerContainersArray[filePointerContainerIndex].fileHandleID;
		if (i < FilePointerInfosBase)
			break;
	}
	return 0;

}

int FileIOManager::SIXB44F0(char* fpath, FileAccessType fileAccessType, Hashes* pHashesStruct, int a4) {

	int resourceID;
	LARGE_INTEGER newFilePointerPosition;

	FilePathContainer* pFilePathContainer = GetFilePathContainerFromPath(fpath);
	if (!pFilePathContainer ) {

		if (pHashesStruct && fileAccessType != CREATE && fileAccessType != MODIFY)
			return 0; // TODO: finish another function and complete

		pFilePathContainer = &DefaultFilePathContainer;

	}

	char joinedFpath[256];
	pFilePathContainer->pathJoiningFunction(pFilePathContainer, joinedFpath, fpath, 256);

	if (pFilePathContainer->pathTypeInfo.status1 == 1)
		return MaximumValidResourceID;

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
				FileHandleContainersArray[fileHandleIndex].bCanFileBeReadNonsequentially = bCanFileBeReadNonsequentially;
		}
	}
	FileHandleContainersArray[fileHandleIndex].pSomething = 0;
	return fileHandleIndex + 1;

}

int FileIOManager::GetAvailableFilePointerInfoIndex() {

	int availableIndex = -1;

	RawEnterCriticalSection(CriticalSectionIndex);
	for (int i = 0; i < MaxFilePointerInfoCount; i++) {
		FilePointerInfo* pFilePointerInfo = &FilePointerInfoArray[i];
		if (!pFilePointerInfo->bIsInUse) {
			availableIndex = i;
			break;
		}
	}
	if (availableIndex != -1)
		FilePointerInfoArray[availableIndex].bIsInUse = 1;
	RawLeaveCriticalSection(CriticalSectionIndex);
	return availableIndex;

}

int FileIOManager::LinkAvailableFilePointerContainerWithHashesStruct(Hashes* pHashesStruct, int hashesStructIndex) {

	FilePointerContainer* filePointerContainersArray = pHashesStruct->filePointerContainersArray;

	RawEnterCriticalSection(CriticalSectionIndex);

	int availableFilePointerContainerIndex = 0;
	for (int i = 0; filePointerContainersArray[i].fileHandleIndex != -1; i++) {
		if (++availableFilePointerContainerIndex >= 8) {
			availableFilePointerContainerIndex = -1;
			goto EXIT;
		}
	}
	filePointerContainersArray[availableFilePointerContainerIndex].fileHandleIndex = hashesStructIndex;

EXIT:
	RawLeaveCriticalSection(CriticalSectionIndex);
	return availableFilePointerContainerIndex;

}

// if pHashesStruct->status <= -2 return SomeLargeInteger+(base*256), else return SomeLargeInteger+base
unsigned __int64 FileIOManager::CalculateStatusDependentValue(Hashes* pHashesStruct, int base) {

	bool flag = pHashesStruct->status <= -2;
	return SomeLargeInteger.QuadPart + ((__int64)base << (flag ? 8 : 0));

}

int FileIOManager::InitializeFilePointerContainerFileHandleID(Hashes* pHashesStruct, int filePointerContainerIndex) {

	// return FilePointerContainer's fileHandleID, if any
	FilePointerContainer* pFilePointerContainer = &pHashesStruct->filePointerContainersArray[filePointerContainerIndex];
	if (pFilePointerContainer->fileHandleID)
		return pFilePointerContainer->fileHandleID;

	// give FilePointerContainer a new fileHandleID and reset its FilePointerPosition to 0
	int fileHandleID = SIXB44F0(const_cast<char*>(pHashesStruct->fileName), (FileAccessType)pHashesStruct->fileAccessType, 0, 0);
	pFilePointerContainer->fileHandleID = fileHandleID;
	pFilePointerContainer->filePointerPosition.QuadPart = 0;
	return fileHandleID;

}

int FileIOManager::SomeLargeFileReadingFunction(Hashes* pHashesStruct, char* fname, FileAccessType fileAccessType) {

	// return if not READ
	if (fileAccessType)
		return 0;

	int someStructIndex = GetFileDataIndex(pHashesStruct, fname);
	if (someStructIndex < 0 || !pHashesStruct->SomeStructArray[someStructIndex].fileDataSize1)
		return 0;

	int hashesStructIndex = GetAvailableFilePointerInfoIndex();
	if (hashesStructIndex == -1)
		return 0;

	int filePointerContainerIndex = LinkAvailableFilePointerContainerWithHashesStruct(pHashesStruct, hashesStructIndex);
	if (filePointerContainerIndex == -1) {
		FilePointerInfoArray[hashesStructIndex].bIsInUse = 0;
		return 0;
	}

	FilePointerContainer* pFilePointerContainer = &pHashesStruct->filePointerContainersArray[filePointerContainerIndex];
	InitializeFilePointerContainerFileHandleID(pHashesStruct, filePointerContainerIndex);
	FilePointerInfoArray[hashesStructIndex].pHashesStruct = pHashesStruct;
	FilePointerInfo* pFilePointerInfo = &FilePointerInfoArray[hashesStructIndex];
	LARGE_INTEGER fileStart = ToLargeInt(CalculateStatusDependentValue(pHashesStruct, pHashesStruct->SomeStructArray[someStructIndex].someNum));
	
	FilePointerInfoArray[hashesStructIndex].fileStartPosition = fileStart;
	FilePointerInfoArray[hashesStructIndex].filePointerPosition = fileStart;
	FilePointerInfoArray[hashesStructIndex].fileDataSize = pHashesStruct->SomeStructArray[someStructIndex].fileDataSize1;
	FilePointerInfoArray[hashesStructIndex].fileDataSizeWhen0x28isNonzero = pHashesStruct->SomeStructArray[someStructIndex].fileDataSize2;
	FilePointerInfoArray[hashesStructIndex].bIsRelative = pHashesStruct->SomeStructArray[someStructIndex].bIsRelative;
	FilePointerInfoArray[hashesStructIndex].filePointerContainerIndex = filePointerContainerIndex;
	pFilePointerContainer->filePointerPosition = fileStart;

	LARGE_INTEGER distToMove;

	while (true) {
		distToMove = FilePointerInfoArray[hashesStructIndex].fileStartPosition;
		if ((SetFilePointer(pFilePointerContainer->fileHandleID, distToMove, FILE_BEGIN).HighPart & GENERIC_READ) == 0LL)
			break;
	}

	if (FilePointerInfoArray[hashesStructIndex].bIsRelative == 2) {
		pSomeFilePointerInfo = &FilePointerInfoArray[hashesStructIndex];
		pSomeFilePointerContainer = pFilePointerContainer;
		SomeFileStartPosition = pFilePointerInfo->fileStartPosition;
		FileDataBufferCharsCount = 0;
	}
	return hashesStructIndex + FilePointerInfosBase;

}

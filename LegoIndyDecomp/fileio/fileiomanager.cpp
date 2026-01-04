#include "fileio.h"
#include "strings/std.h"

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
		dwDesiredAccess = 0x80000000; // GENERIC_READ
		dwCreationDisposition = 4; // OPEN_EXISTING
		goto ACCESS_TYPE_INITIALIZED;
	}

	if (fileAccessType == FileAccessType::CREATE) {
		dwDesiredAccess = 0x40000000; // GENERIC_WRITE
		dwCreationDisposition = 2; // CREATE_ALWAYS
		goto ACCESS_TYPE_INITIALIZED;
	}

	if (fileAccessType == FileAccessType::MODIFY) {
		dwDesiredAccess = 0x40000000; // GENERIC_WRITE
		dwCreationDisposition = 4; // OPEN_ALWAYS
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

int FileIOManager::SetFilePointer(FilePointerInfo* pFilePointerInfo, LARGE_INTEGER distToMove, DWORD moveMethod) {

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

	LARGE_INTEGER filePointerPosition;
	filePointerPosition.LowPart = SetFilePointer(
		pFilePointerContainer->fileHandleID,
		newPosition,
		FILE_BEGIN
	);
	pFilePointerInfo->filePointerPosition.QuadPart = filePointerPosition.QuadPart;
	pFilePointerContainer->filePointerPosition.LowPart = filePointerPosition.LowPart;
	pFilePointerContainer->filePointerPosition.HighPart = pFilePointerInfo->filePointerPosition.HighPart;
	return pFilePointerInfo->filePointerPosition.QuadPart;

}

int FileIOManager::SetFilePointer(FileBufferContainer* pFileBufferContainer, LARGE_INTEGER distToMove, DWORD moveMethod) {

	switch(moveMethod) {
		case FILE_CURRENT: {
			pFileBufferContainer->filePointerPosition += distToMove.QuadPart;
			return pFileBufferContainer->filePointerPosition - pFileBufferContainer->textBuffer;
		}
		case FILE_END: {
			char* newFilePointer = &pFileBufferContainer->textBufferEnd[-distToMove.QuadPart];
			pFileBufferContainer->filePointerPosition = newFilePointer;
			return newFilePointer - pFileBufferContainer->textBuffer;
		}
		case FILE_BEGIN:
		default: {
			pFileBufferContainer->filePointerPosition = &pFileBufferContainer->textBuffer[distToMove.QuadPart];
			return distToMove.LowPart;
		}
	}

}

int FileIOManager::SetFilePointer(FileHandleContainer* pFileHandleContainer, LARGE_INTEGER distToMove, DWORD moveMethod) {

	if (!pFileHandleContainer->bCanFileBeReadNonsequentially)
		return RawSetFilePointer(pFileHandleContainer->fileHandleIndex, distToMove, moveMethod).LowPart;

	switch (moveMethod) {
		case FILE_CURRENT: {
			pFileHandleContainer->filePointerPosition.QuadPart += distToMove.QuadPart;
			return pFileHandleContainer->filePointerPosition.LowPart;
		}
		case FILE_END: {
			pFileHandleContainer->filePointerPosition.QuadPart = pFileHandleContainer->fileEndPosition.QuadPart - distToMove.QuadPart;
			return pFileHandleContainer->filePointerPosition.LowPart;
		}
		case FILE_BEGIN:
		default: {
			pFileHandleContainer->filePointerPosition.QuadPart = distToMove.QuadPart;
			return pFileHandleContainer->filePointerPosition.LowPart;
		}
	}

}

int FileIOManager::SetFilePointer(int resourceID, LARGE_INTEGER distToMove, DWORD moveMethod) {

	if (resourceID >= MaximumValidResourceID)
		return 0;

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

	for (FilePathContainer* pFilePathContainer : FilePathContainersArray) {
		if (!_strncmp(fpath, pFilePathContainer->absolutePath, pFilePathContainer->pathLength ) )
			return pFilePathContainer;
	}

	return 0;

}

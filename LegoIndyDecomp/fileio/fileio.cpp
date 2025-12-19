#include "fileio.h"

// temporary initialization of critical section
FileIOManager::FileIOManager() {
	CriticalSectionsArray[0] = new CRITICAL_SECTION();
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

	int v5;

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

	int v4 = 0;
	while (FileHandlesArray[v4] != (HANDLE)-1) {
		if (++v4 >= 64) {
			v5 = -1;
			goto FILE_HANDLES_ARRAY_FULL;
		}
	}
	v5 = v4;

FILE_HANDLES_ARRAY_FULL:

	FileHandlesArray[v5] = CreateFileA(fpath, dwDesiredAccess, 1, 0, dwCreationDisposition, 0, 0);
	if (CriticalSectionIndex - 1 <= 11)
		LeaveCriticalSection(CriticalSectionsArray[CriticalSectionIndex]);
	if (FileHandlesArray[v5] == (HANDLE)-1)
		return -1;

	return v5;

}

bool FileIOManager::CloseFileHandle(int fileHandleIndex) {

	bool wasClosed = CloseHandle(FileHandlesArray[fileHandleIndex]);
	FileHandlesArray[fileHandleIndex] = (HANDLE)-1;
	return !wasClosed;

}


void FileIOManager::CloseFileHandleID(int fileHandleID) {

	if (fileHandleID >= 4096)
		return;

	if (fileHandleID >= 1024) {
		// TODO: some other handle closing function()
		return;
	}

	int fileHandleIndex = fileHandleID - 1;
	while (CloseFileHandle(fileHandleIndex) < 0);

	FileHandleContainer* pFileHandleContainer = &FileHandleContainersArray[fileHandleIndex];
	FileDataContainer* pFileDataContainer = pFileHandleContainer->pFileDataContainer;

	if (pFileDataContainer)
		pFileDataContainer->pFileHandleContainer = 0;

	memset(pFileHandleContainer, 0, sizeof(FileHandleContainer));

}

int FileIOManager::Write(int fileHandleIndex, LPVOID lpBuffer, int numberOfBytesToWrite) {

	HANDLE hFile = FileHandlesArray[fileHandleIndex];
	DWORD numberOfBytesWritten = 0;
	bool success = WriteFile(hFile, lpBuffer, numberOfBytesToWrite, &numberOfBytesWritten, 0);
	return success ? numberOfBytesWritten : 0;

}

int FileIOManager::Read(int fileHandleIndex, LPVOID lpBuffer, int numberOfBytesToRead) {

	HANDLE hFile = FileHandlesArray[fileHandleIndex];
	DWORD numberOfBytesRead = 0;
	ReadFile(hFile, lpBuffer, numberOfBytesToRead, &numberOfBytesRead, 0);
	return numberOfBytesRead;

}

LARGE_INTEGER FileIOManager::MoveFilePointer(int fileHandleIndex, LARGE_INTEGER distToMove, int moveMethod) {

	LARGE_INTEGER newFilePointer;

	if (moveMethod && moveMethod != FILE_CURRENT)
		moveMethod = FILE_END;

	newFilePointer.QuadPart = 0;

	if (SetFilePointerEx(FileHandlesArray[fileHandleIndex], distToMove, &newFilePointer, moveMethod))
		return newFilePointer;

	newFilePointer.QuadPart = -1;
	return newFilePointer;

}

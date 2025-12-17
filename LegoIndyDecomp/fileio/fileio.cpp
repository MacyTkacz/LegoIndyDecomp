#include "fileio.h"

static FileIOManager* FileIOManager::Instance() {
	if (!_instance)
		_instance = new FileIOManager();
	return _instance;
}

int FileIOManager::InitializeNewCriticalSection() {
	if (NumberOfCriticalSections > 12)
		return 0;
	NumberOfCriticalSections++;
	InitializeCriticalSection(CriticalSectionsArray[NumberOfCriticalSections]);
	CriticalSectionLockCount = CriticalSectionsArray[NumberOfCriticalSections].LockCount;
	return NumberOfCriticalSections;
}

int FileIOManager::CreateFileHandle(LPCSTR fpath, FileAccessType fileAccessType) {

	int v4;
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
		CriticalSectionIndex = InitializeNewCriticalSection();
	if (CriticalSectionIndex - 1 <= 11)
		EnterCriticalSection(CriticalSectionsArray[CriticalSectionIndex]);

	v4 = 0;
	while (FileHandlesArray[i] != (HANDLE)-1) {
		if (++i >= 64) {
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

int FileIOManager::Write(int fileHandleIndex, LPCVOID lpBuffer, int numberOfBytesToWrite) {

	HANDLE hFile = FileHandlesArray[fileHandleIndex];
	int numberOfBytesWritten = 0;
	bool success = WriteFile(hFile, lpBuffer, numberOfBytesToWrite, &numberOfBytesWritten, 0);
	return success ? numberOfBytesWritten : 0;

}

int FileIOManager::Read(int fileHandleIndex, LPCVOID lpBuffer, int numberOfBytesToRead) {

	HANDLE hFile = FileHandlesArray[fileHandleIndex];
	int numberOfBytesRead = 0;
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

	return (LARGE_INTEGER)-1LL;

}

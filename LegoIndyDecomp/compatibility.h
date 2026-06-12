#ifndef LEGOINDY_COMPATIBILITY_H
#define LEGOINDY_COMPATIBILITY_H

#include <cstdint>

typedef union _LARGE_INTEGER {
  struct {
    unsigned long LowPart;
    long HighPart;
  };
  long long QuadPart;
} LARGE_INTEGER;

struct CRITICAL_SECTION {
    long LockCount;
};

typedef void* HANDLE;
typedef void* LPVOID;

const uint64_t GENERIC_READ = 0x80000000;
const uint64_t GENERIC_WRITE = 0x40000000;
const uint64_t OPEN_EXISTING = 3;
const uint64_t CREATE_ALWAYS = 2;
const uint64_t OPEN_ALWAYS = 4;
const uint64_t FILE_CURRENT = 1;
const uint64_t FILE_BEGIN = 0;
const uint64_t FILE_END = 2;

void InitializeCriticalSection(CRITICAL_SECTION* cs);
void EnterCriticalSection(CRITICAL_SECTION* cs);
void LeaveCriticalSection(CRITICAL_SECTION* cs);
bool CloseHandle(HANDLE h);
bool WriteFile(HANDLE hFile, LPVOID lpBuffer, int numberOfBytesToWrite, unsigned long* numberOfBytesWritten, void* lpOverlapped);
bool ReadFile(HANDLE hFile, LPVOID lpBuffer, int numberOfBytesToRead, unsigned long* numberOfBytesRead, void* lpOverlapped);
bool SetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistanceToMove, LARGE_INTEGER* lpNewFilePointer, int dwMoveMethod);

HANDLE CreateFileA(const char* fpath, uint64_t dwDesiredAccess, uint64_t dwShareMode, void* lpSecurityAttributes, uint64_t dwCreationDisposition, uint64_t dwFlagsAndAttributes, HANDLE hTemplateFile);

#endif // LEGOINDY_COMPATIBILITY_H
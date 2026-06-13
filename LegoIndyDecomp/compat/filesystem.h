#ifndef LEGOINDY_FILESYSTEM_H
#define LEGOINDY_FILESYSTEM_H

#ifdef _WIN32
 #include <Windows.h>
#endif

#include <cstdint>
#include <memory>

namespace FileSystem {

bool CreateDirectory(const char* path);
bool Exists(const char* path);

enum KnownPath { DOCUMENTS, LOCAL_DATA };
const char* GetKnownPath( KnownPath path );

struct FileHandle {
#ifdef _WIN32
    HANDLE value;
#else
    int64_t value;
#endif
};

enum FilePosition { CURRENT, START, END };
enum class FileAccessType { READ=1, WRITE=1<<1 };
enum class FileShareType { NONE=1, READ=1<<1 };
enum FileCreateMode { CREATE_NEW=1, CREATE_ALWAYS, OPEN_EXISTING, OPEN_ALWAYS, TRUNCATE_EXISTING };
enum FileAttribute { NORMAL=1 };
class File {
public:

    File(FileHandle handle, uint8_t accessType, uint8_t shareType);

    bool SetPointer( FilePosition position );
    bool SetPointer( uint64_t position );
    bool SetEOF();
    bool Save();
    bool Read( void* dest, uint64_t bytesToRead, uint64_t* bytesRead );
    bool Write( void* source, uint64_t bytesToWrite, uint64_t* bytesWritten );

    // in win32, these functions all use info from GetFileAttributesExA
    uint64_t GetSize();
    uint64_t GetAttributes();
    uint64_t GetCreationTime();
    uint64_t GetLastAccessTime();
    uint64_t GetLastWriteTime();

private:

    FileHandle handle;
    uint64_t pointer;
    uint8_t accessType;
    uint8_t shareType;

};
// creates a new / retrieves an existing file
std::shared_ptr<File> GetFile( const char* path, uint8_t accessType, uint8_t shareType, FileCreateMode createMode, uint64_t attributes );

class Search {
public:
    bool FindNext();
    std::shared_ptr<File> Get();
private:
    const char searchPath[1024];
    std::shared_ptr<File> match;
};
std::unique_ptr<Search> FindFile( const char* searchPath );

bool MoveFile( const char* existingPath, const char* newPath );
bool DeleteFile( const char* path );

}; // namespace FileSystem

#endif // LEGOINDY_FILESYSTEM_H
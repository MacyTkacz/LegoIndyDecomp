#ifdef _WIN32
 #include <utils.h>
 #include <bit>
#else
 #include <unistd.h>
 #include <fcntl.h>
#endif

#include <filesystem>
#include <error.h>

#include <filesystem.h>
#include "translate.h"

using namespace FileSystem;

bool FileSystem::Exists(const char* path) { return std::filesystem::exists(path); }
bool FileSystem::CreateDirectory(const char* path) { return std::filesystem::create_directory(path); }
bool FileSystem::DeleteFile( const char* path ) { return std::filesystem::remove(path); }
uint64_t FileSystem::GetFileSize(const char* path) { return std::filesystem::file_size(path); };

// UNIMPLEMENTED
const char* FileSystem::GetKnownPath( KnownPath path ) { throw NotImplemented(); };
bool FileSystem::MoveFile( const char* existingPath, const char* newPath ) { throw NotImplemented(); };
std::unique_ptr<Search> FindFile( const char* searchPath ) { throw NotImplemented(); };
bool FileSystem::File::SetEOF() { throw NotImplemented(); };

std::unique_ptr<File> FileSystem::GetFile( const char* path, FileAccessType accessType, FileShareType shareType, FileCreateMode createMode, FileAttribute attributes ) {
    FileHandle hFile;
#ifdef _WIN32

    uint64_t _attributes = 0;
    if (attributes&FileAttribute::NORMAL) _attributes |= FILE_ATTRIBUTE_NORMAL;

    uint64_t _accessType = To<FileAccessType>(accessType);
    if (!_accessType) return nullptr;

    uint8_t _shareType = To<FileShareType>(shareType);
    uint8_t _createMode = To<FileCreateMode>(createMode);

    HANDLE h = CreateFileA( path, _accessType, _shareType, nullptr, _createMode, _attributes, 0);
    if (h==INVALID_HANDLE_VALUE) return nullptr;

    hFile = h;

#else

    uint64_t flags = 0;
    flags |= To<FileAccessType>(accessType);
    flags |= To<FileCreateMode>(createMode);

    mode_t mode = 0;
    mode |= To<FileShareType>(shareType);

    int64_t fd = open( path, flags, mode );
    if (fd == -1) return nullptr;

    hFile = fd;

#endif
    auto pFile = std::make_unique<File>( hFile, accessType, shareType );
    return pFile;
}

FileSystem::File::File(FileHandle handle, FileAccessType accessType, FileShareType shareType) {
    m_handle = handle;
    m_accessType = accessType;
    m_shareType = shareType;
}

FileSystem::File::~File() {
#ifdef _WIN32
    if (m_handle != INVALID_HANDLE_VALUE)
        CloseHandle(m_handle);
#else
    if (m_handle >= 0)
        close(m_handle);
#endif
}

// safely transfers file handle, preventing duplication
FileSystem::File::File(File&& other) : m_handle(other.m_handle) {
#ifdef _WIN32
    other.m_handle = INVALID_HANDLE_VALUE;
#else
    other.m_handle = -1;
#endif
}

bool FileSystem::File::SetPointer( FilePosition position, int64_t* newPosition ) {
    if (position == FilePosition::CURRENT) return false;
#ifdef _WIN32
    uint8_t moveMethod = ( position == FilePosition::START ? FILE_BEGIN : FILE_END );
    LARGE_INTEGER* pliNewPosition = nullptr;
    if (newPosition)
        pliNewPosition = std::bit_cast<LARGE_INTEGER*>( newPosition );
    return SetFilePointerEx(m_handle,ToLargeInt(0),pliNewPosition,moveMethod);
#else
    uint8_t whence = ( position == FilePosition::START ? SEEK_SET : SEEK_END );
    int64_t offset = lseek(m_handle,0,whence);
    if (newPosition)
        *newPosition = offset;
    return offset != -1;
#endif
}

bool FileSystem::File::SetPointer( uint64_t position, int64_t* newPosition ) {
#ifdef _WIN32
    uint8_t moveMethod = FILE_BEGIN;
    LARGE_INTEGER* pliNewPosition = nullptr;
    if (newPosition)
        pliNewPosition = std::bit_cast<LARGE_INTEGER*>( newPosition );
    return SetFilePointerEx(m_handle,ToLargeInt(position),pliNewPosition,moveMethod);
#else
    int64_t offset = lseek(m_handle,position,SEEK_SET);
    if (newPosition)
        *newPosition = offset;
    return offset != -1;
#endif
}

bool FileSystem::File::SetPointer( uint64_t distToMove, FilePosition moveMethod, int64_t* newPosition ) {
#ifdef _WIN32
    uint8_t _moveMethod;
    switch(moveMethod) {
        case FilePosition::CURRENT: _moveMethod = FILE_CURRENT; break; 
        case FilePosition::START: _moveMethod = FILE_BEGIN; break; 
        case FilePosition::END: _moveMethod = FILE_END; break; 
        default: return -1;
    }
    LARGE_INTEGER* pliNewPosition = nullptr;
    if (newPosition)
        pliNewPosition = std::bit_cast<LARGE_INTEGER*>( newPosition );
    return SetFilePointerEx(m_handle,ToLargeInt(distToMove),pliNewPosition,_moveMethod);
#else
    uint8_t whence;
    switch(moveMethod) {
        case FilePosition::CURRENT: whence = SEEK_CUR; break; 
        case FilePosition::START: whence = SEEK_SET; break; 
        case FilePosition::END: whence = SEEK_END; break; 
        default: return false;
    }
    int64_t offset = lseek(m_handle,distToMove,whence);
    if (newPosition)
        *newPosition = offset;
    return offset != -1;
#endif
}

bool FileSystem::File::Save() {
#ifdef _WIN32
    return FlushFileBuffers(m_handle);
#else
    return fsync(m_handle) == 0;
#endif
}

bool FileSystem::File::Write( const void* source, uint32_t bytesToWrite, unsigned long* bytesWritten ) {
#ifdef _WIN32
    LPDWORD _bytesWritten = bytesWritten ? static_cast<LPDWORD>(bytesWritten) : nullptr;
    bool success = WriteFile(m_handle,source,bytesToWrite,_bytesWritten,nullptr);
    return success;
#else
    int64_t _bytesWritten = write(m_handle,source,bytesToWrite);
    if (bytesWritten)
        *bytesWritten = _bytesWritten != -1 ? _bytesWritten : 0;
    return _bytesWritten != -1;
#endif
}
bool FileSystem::File::Write( const void* source, uint32_t bytesToWrite ) { return this->Write(source,bytesToWrite,nullptr); }

bool FileSystem::File::Read( void* source, uint32_t bytesToRead, unsigned long* bytesRead ) {
#ifdef _WIN32
    LPDWORD _bytesRead = bytesRead ? static_cast<LPDWORD>(bytesRead) : nullptr;
    return ReadFile(m_handle,source,bytesToRead,_bytesRead,nullptr);
#else
    int64_t _bytesRead = read(m_handle,source,bytesToRead);
    if (bytesRead)
        *bytesRead = _bytesRead != -1 ? _bytesRead : 0;
    return _bytesRead != -1;
#endif
}
bool FileSystem::File::Read( void* source, uint32_t bytesToRead ) { return this->Read(source,bytesToRead,nullptr); }
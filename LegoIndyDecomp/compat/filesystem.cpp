#ifndef _WIN32
 #include <unistd.h>
 #include <fcntl.h>
#endif

#include <filesystem>
#include "filesystem.h"
#include "error.h"

using namespace FileSystem;

bool FileSystem::Exists(const char* path) { return std::filesystem::exists(path); }
bool FileSystem::CreateDirectory(const char* path) { return std::filesystem::create_directory(path); }
bool FileSystem::DeleteFile( const char* path ) { return std::filesystem::remove(path); }

// UNIMPLEMENTED
const char* FileSystem::GetKnownPath( KnownPath path ) { throw NotImplemented(); };
bool FileSystem::MoveFile( const char* existingPath, const char* newPath ) { throw NotImplemented(); };
std::unique_ptr<Search> FindFile( const char* searchPath ) { throw NotImplemented(); };

std::shared_ptr<File> FileSystem::GetFile( const char* path, uint8_t accessType, uint8_t shareType, uint8_t createMode, uint64_t attributes ) {
    FileHandle hFile;
#ifdef _WIN32

    uint64_t _accessType = 0;
    if (accessType&static_cast<uint64_t>(FileAccessType::READ)) _accessType |= GENERIC_READ;
    if (accessType&static_cast<uint64_t>(FileAccessType::WRITE)) _accessType |= GENERIC_WRITE;
    if (!_accessType) return nullptr;

    uint8_t _shareType = 0;
    if (shareType&static_cast<uint8_t>(FileShareType::READ)) _shareType |= FILE_SHARE_READ;

    uint64_t _attributes = 0;
    if (attributes&FileAttribute::NORMAL) _attributes |= FILE_ATTRIBUTE_NORMAL;

    HANDLE h = CreateFileA( path, _accessType, _shareType, 0, createMode, _attributes, 0);
    if (h==INVALID_HANDLE_VALUE) return nullptr;

    hFile.value = h;

#else

    uint64_t flags = 0;

    uint8_t _accessType = 0;
    if (accessType&static_cast<uint8_t>(FileAccessType::WRITE)) {
        if (accessType&static_cast<uint8_t>(FileAccessType::READ)) {
            _accessType = O_RDWR;
            goto END_ACCESS_TYPE;
        }
        _accessType = O_WRONLY;
        goto END_ACCESS_TYPE;
    }
    _accessType = O_RDONLY;
END_ACCESS_TYPE:
    flags |= _accessType;

    uint8_t _createMode = 0;
    switch(createMode) {
    case FileCreateMode::CREATE_NEW: flags |= (O_CREAT|O_EXCL); break;
    case FileCreateMode::CREATE_ALWAYS: {
        if ( attributes&FileAttribute::NORMAL && ( attributes&FileAttribute::HIDDEN || attributes&FileAttribute::SYSTEM ))     
            return nullptr;
        flags |= (O_CREAT | O_TRUNC);
        break;
    }
    case FileCreateMode::OPEN_ALWAYS: flags |= O_CREAT; break;
    case FileCreateMode::TRUNCATE_EXISTING: flags |= O_TRUNC; break;
    }

    // assigns user file permissions (may change from user to group or others, not sure yet)
    mode_t mode = 0;
    if (shareType&static_cast<uint8_t>(FileShareType::NONE)) mode |= S_IRGRP; // don't have a good solution for "no access until file handle closed" yet
    if (shareType&static_cast<uint8_t>(FileShareType::READ)) mode |= S_IRGRP;

    // more attributes will be accounted for as they're implemented

    int64_t fd = open( path, flags, mode);
    if (fd == -1) return nullptr;

    hFile.value = fd;

#endif
    auto pFile = std::make_shared<File>( hFile, accessType, shareType );
    return pFile;
}

FileSystem::File::File(FileHandle handle, uint8_t accessType, uint8_t shareType) {
    this->handle = handle;
    this->accessType = accessType;
    this->shareType = shareType;
}
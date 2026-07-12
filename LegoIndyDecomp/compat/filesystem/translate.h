#ifndef LEGOINDY_FILESYSTEM_TRANSLATE_H
#define LEGOINDY_FILESYSTEM_TRANSLATE_H

#ifdef _WIN32
#else
 #include <fcntl.h>
#endif

#include <type_traits>
#include "filesystem.h"
#include "utils.h"

template <typename T, typename V>
constexpr bool HasFlag(T value, V flag) { return ( static_cast<uint64_t>(value)&static_cast<uint64_t>(flag) ) != 0; }

template <typename T>
constexpr uint64_t _FileAccessType(T accessType) {
    const bool read = HasFlag(accessType,FileSystem::FileAccessType::READ);
    const bool write = HasFlag(accessType,FileSystem::FileAccessType::WRITE);
#ifdef _WIN32
    return ( read ? GENERIC_READ : 0 ) | ( write ? GENERIC_WRITE : 0 );
#else
    if (read && write) return O_RDWR;
    if (read) return O_RDONLY;
    return O_WRONLY;
#endif
}

template <typename T>
constexpr uint16_t _FileShareType(T shareType) {
    const bool read = HasFlag(shareType,FileSystem::FileShareType::READ);
    const bool write = HasFlag(shareType,FileSystem::FileShareType::WRITE);
#ifdef _WIN32
    if (!static_cast<uint8_t>(shareType)) return 0;
    uint16_t _shareType = 0;
    if (read) _shareType |= FILE_SHARE_READ;
    if (write) _shareType |= FILE_SHARE_WRITE;
    return _shareType;
#else
    if (!static_cast<uint8_t>(shareType)) return S_IRUSR;
    uint16_t _shareType = 0;
    if (read) _shareType |= S_IRUSR;
    if (write) _shareType |= S_IWUSR;
    return _shareType;
#endif
}

template <typename T>
constexpr uint16_t _FileCreateMode(T createMode) {
#ifdef _WIN32
    return static_cast<uint16_t>(createMode);
#else
    switch(createMode) {
    case FileSystem::FileCreateMode::_CREATE_NEW:
        return O_CREAT|O_EXCL;
    case FileSystem::FileCreateMode::_CREATE_ALWAYS:
        return O_CREAT|O_TRUNC;
    case FileSystem::FileCreateMode::_OPEN_ALWAYS:
        return O_CREAT;
    case FileSystem::FileCreateMode::_TRUNCATE_EXISTING:
        return O_TRUNC;
    default:
        return 0;
    }
#endif
}

namespace FileSystem {

// properly routes an enum type to its corresponding underscore-prefixed function
#define HANDLE_ENUM(enum) \
if ( std::is_same<T,FileSystem::enum>::value ) \
    return static_cast<uint64_t>( _##enum( static_cast<FileSystem::enum>(value) ) );

// convert enum to platform-specific value (Win32, Unix)
template <typename T>
constexpr uint64_t To(T value) {
    HANDLE_ENUM(FileAccessType)
    HANDLE_ENUM(FileShareType)
    HANDLE_ENUM(FileCreateMode)
}

#undef HANDLE_ENUM

}

#endif // LEGOINDY_FILESYSTEM_TRANSLATE_H
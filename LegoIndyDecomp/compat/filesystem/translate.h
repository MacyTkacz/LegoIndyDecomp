#ifndef LEGOINDY_FILESYSTEM_TRANSLATE_H
#define LEGOINDY_FILESYSTEM_TRANSLATE_H

#ifdef _WIN32
#else
 #include <fcntl.h>
#endif

#include <type_traits>
#include "filesystem.h"

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
#ifdef _WIN32
    if (!shareType) return 0;
    uint16_t _shareType = 0;
    if (HasFlag(shareType,FileSystem::FileShareType::READ)) _shareType |= FILE_SHARE_READ;
    if (HasFlag(shareType,FileSystem::FileShareType::WRITE)) _shareType |= FILE_SHARE_WRITE;
    return _shareType;
#else
    return S_IRUSR;
#endif
}

template <typename T>
constexpr uint16_t _FileCreateMode(T createMode) {
#ifdef _WIN32
    return static_cast<uint16_t>(createMode);
#else
    if ( HasFlag(createMode,FileSystem::FileCreateMode::_CREATE_NEW) )
        return O_CREAT|O_EXCL;
    if ( HasFlag(createMode,FileSystem::FileCreateMode::_CREATE_ALWAYS) )
        return O_CREAT|O_TRUNC;
    if ( HasFlag(createMode,FileSystem::FileCreateMode::_OPEN_ALWAYS) )
        return O_CREAT;
    if ( HasFlag(createMode,FileSystem::FileCreateMode::_TRUNCATE_EXISTING) )
        return O_TRUNC;
    return 0;
#endif
}

namespace FileSystem {

// convert enum to platform-specific value (Win32, Unix)
template <typename Target, typename Value>
constexpr uint64_t To(Value value) {
    if ( std::is_same<Target,FileSystem::FileAccessType>::value )
        return static_cast<uint64_t>(_FileAccessType(value));
    if ( std::is_same<Target,FileSystem::FileShareType>::value )
        return static_cast<uint64_t>(_FileShareType(value));
    if ( std::is_same<Target,FileSystem::FileCreateMode>::value )
        return static_cast<uint64_t>(_FileCreateMode(value));
}

}

#endif // LEGOINDY_FILESYSTEM_TRANSLATE_H
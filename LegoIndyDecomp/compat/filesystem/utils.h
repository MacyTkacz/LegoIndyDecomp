#ifndef LEGOINDY_FILESYSTEM_UTILS_H
#define LEGOINDY_FILESYSTEM_UTILS_H

// properly routes an enum type to its corresponding underscore-prefixed function
#define HANDLE_ENUM(enum) \
if ( std::is_same<T,FileSystem::enum>::value ) \
    return static_cast<uint64_t>( _##enum( static_cast<FileSystem::enum>(value) ) );

#endif // LEGOINDY_FILESYSTEM_UTILS_H
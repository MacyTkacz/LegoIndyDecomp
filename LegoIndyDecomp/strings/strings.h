#ifndef LEGOINDY_STRINGS_H
#define LEGOINDY_STRINGS_H

// ====================== STRUCTS ======================

enum PathType { UPPERCASE, MIXEDCASE };

struct PathTypeInfo {
	char lastCharWritten;
	char padding[15];
	char separator;
};

struct FilePathContainer {
	PathTypeInfo pathTypeInfo; // 0x00
	char pad1[35]; // 0x11
	char path[16]; // 0x34
	int pathLength; // 0x44
	char pad2[4]; // 0x48
	char drivePrefix[16]; // 0x4C (size is a guess)
	char pad3[16]; // 0x5C
	char someStr[32]; // 0x6C
	char pad4[32]; // 0x8C
	char relativePath[16]; // 0xAC
	char pad5[368]; // 0xBC
	int (__cdecl* pathJoiningFunction)(FilePathContainer*, char* fpath_out, char* fpath_in, int size); // 0x22C
	int (__cdecl* func2)(FilePathContainer*); // 0x230
};

// ======================= DATA ========================

inline PathType GlobalPathType = PathType::MIXEDCASE;

// ===================== FUNCTIONS =====================

// mutates path, replacing every instance of a back or forward slash with pathTypeInfo->separator ('\' if pathTypeInfo is NULL)
char* __cdecl MakePathUniform(PathTypeInfo* pathTypeInfo, char* path);

// returns pointer into str at first found instance of starts_with (inclusive)
char* __cdecl GetStringStartingWith(char* str, const char* starts_with);

// resolves a single "/../" in a path, e.g. "C:/files/../a.txt" -> "C:/a.txt"
char* __cdecl ResolveRelativePathSpecifier(PathTypeInfo* pPathTypeInfo, char* path);

//if fpath is absolute:
//   OUTPUT fpath
//if fpath and structPath are the same:
//   OUTPUT [DRIVE:/][relative/path/]
//if fpath begins with structPath:
//   if structPath ends with a slash:
//      OUTPUT [DRIVE:/][relative/path/]fpath
//   else:
//      OUTPUT fpath
//OUTPUT [DRIVE:/][relative/path/]fpath
int __cdecl JoinPath(FilePathContainer* pFilePathContainer, char* fname, char* fpath, int maxLength);

#endif // LEGOINDY_STRINGS_H

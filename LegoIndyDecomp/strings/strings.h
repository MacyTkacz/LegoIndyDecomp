#ifndef LEGOINDY_STRINGS_H
#define LEGOINDY_STRINGS_H

// ====================== STRUCTS ======================

struct PathTypeInfo {
	char lastCharWritten;
	char padding[15];
	char separator;
};

enum PathType { UPPERCASE, MIXEDCASE };

// ======================= DATA ========================

inline PathType GlobalPathType = PathType::MIXEDCASE;

// ===================== FUNCTIONS =====================

// mutates path, replacing every instance of a back or forward slash with pathTypeInfo->separator ('\' if pathTypeInfo is NULL)
char* __cdecl MakePathUniform(PathTypeInfo* pathTypeInfo, char* path);

// returns pointer into str at first found instance of starts_with (inclusive)
char* __cdecl GetStringStartingWith(char* str, const char* starts_with);

// case-insensitive strcmp
int __cdecl _strcmpi(char* str1, char* str2);

#endif // LEGOINDY_STRINGS_H

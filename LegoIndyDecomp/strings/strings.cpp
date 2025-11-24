#include "strings.h"
#include <strings/std.h>

char *MakePathUniform(PathTypeInfo *pathTypeInfo, char *path) {
  char *current_path_offset; // eax
  char separating_char; // bl
  char *current_write_offset; // ecx

  current_path_offset = &pathTypeInfo->lastCharWritten;
  if (pathTypeInfo)
      separating_char = pathTypeInfo->separator;
  else
    separating_char = 92;                       // '\'
  if ( GlobalPathType ) {
    for ( current_path_offset = path; *current_path_offset; ++current_path_offset ) {
      // '/' or '\'
      if ( *current_path_offset == 92 || *current_path_offset == 47 )
        *current_path_offset = separating_char;
    }
  }
  else {
    for ( current_write_offset = path; *current_write_offset; ++current_write_offset ) {
      *current_path_offset = *current_write_offset;
      switch ( *current_write_offset ) {
        case '/':
        case '\\':
          *current_write_offset = separating_char;
          break;
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
          *current_path_offset = *current_path_offset - 32;
          *current_write_offset = *current_path_offset;
          break;
        default:
          continue;
      }
    }
  }
  return current_path_offset;
}

char *GetStringStartingWith(char *str, const char *starts_with) {
  char *result; // eax
  int offset_into_string; // esi
  char *current_separator_offset; // ecx
  char current_separator_char; // dl

  char* target = const_cast<char*>(starts_with);

  result = str;
  if ( !*str )
    return 0;
  for ( offset_into_string = str - target; ; ++offset_into_string )
  {
    current_separator_offset = target;
    if ( *target )
    {
      while ( 1 )
      {
        current_separator_char = current_separator_offset[offset_into_string];
        if ( !current_separator_char || current_separator_char != *current_separator_offset )
          break;
        if ( !*++current_separator_offset )
          return result;
      }
    }
    if ( !*current_separator_offset )
      break;
    if ( !*++result )
      return 0;
  }
  return result;
}

char* ResolveRelativePathSpecifier(PathTypeInfo* pPathTypeInfo, char* path) {

    char* result;
    char* currentPathOffset;
    char separator;
    char currentBacktrackChar;

    result = path;
    for (currentPathOffset = path; *currentPathOffset; ++result) {
        
        separator = pPathTypeInfo->separator;
        if (currentPathOffset[0] == separator &&
            currentPathOffset[1] == '.' &&
            currentPathOffset[2] == '.' &&
            currentPathOffset[3] == separator)
        {
            result = currentPathOffset;

            if (currentPathOffset > path) {
                while (1) {
                    currentBacktrackChar = *--result;
                    if (currentBacktrackChar == separator) {
                        currentPathOffset += 3;
                        goto LABEL_13;
                    }
                    if (currentBacktrackChar == ':')
                        break;
                    if (result <= path)
                        goto LABEL_13;
                }
                ++result;
                currentPathOffset += 4;
            }
        }
LABEL_13:
        *result = *currentPathOffset++;
    }
    *result = 0;
    return result;
}

int JoinPath(FilePathContainer* pFilePathContainer, char* fpath_out, char* fpath_in, int size) {

    char* structPath;
    char* structPathOffsetIntoPath;
    char structPathOffsetIntoPathChar;
    char charBuffer[512];
    int structStr1Length;
    int structPathLength;

    structPath = pFilePathContainer->path;
    structPathLength = _strlen(pFilePathContainer->path);

    if (GetStringStartingWith(fpath_in, ":"))
        goto PATH_IS_ABSOLUTE;

    if (!_strncmp(fpath_in, structPath, structPathLength)) {

        // fpath_in is relative and the same as structPath
        structPathLength = _strlen(structPath);
		structPathOffsetIntoPathChar = fpath_in[structPathLength];
		structPathOffsetIntoPath = &fpath_in[structPathLength];

		if (structPathOffsetIntoPathChar == '\\' || structPathOffsetIntoPathChar == '/') {
PATH_IS_ABSOLUTE:
			_strcpy(charBuffer, fpath_in);
			goto LABEL_8;
		}

		_strcpy(charBuffer, pFilePathContainer->drivePrefix);
		_strcat(charBuffer, pFilePathContainer->relativePath);
		_strcat(charBuffer, structPathOffsetIntoPath);

    }
    else {
        
        _strcpy(charBuffer, pFilePathContainer->drivePrefix);
        _strcat(charBuffer, pFilePathContainer->relativePath);
        _strcat(charBuffer, fpath_in);

    }

LABEL_8:
    structStr1Length = _strlen(pFilePathContainer->drivePrefix);
    MakePathUniform(&pFilePathContainer->pathTypeInfo, &charBuffer[structStr1Length]);
    ResolveRelativePathSpecifier(&pFilePathContainer->pathTypeInfo, charBuffer);

    if (_strlen(charBuffer) >= size)
        return 0;

    _strcpy(fpath_out, charBuffer);

    return 1;

}

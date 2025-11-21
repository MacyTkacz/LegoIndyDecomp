#include "strings.h"

char *__cdecl MakePathUniform(PathTypeInfo *pathTypeInfo, char *path) {
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

char *__cdecl GetStringStartingWith(char *str, const char *starts_with) {
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

int __cdecl _strcmpi(char* str1, char* str2) {

    char str1_current_char;
    char str2_current_char;

    if (!str1)
        return -1;
    if (str2) {
        while (1) {
            str1_current_char = *str1;
            if ( (*str1 - 97) <= 0x19u || str1_current_char >= 0xE0u )
                str1_current_char -= 32;
            str2_current_char = *str2;
            if ( (*str2 - 97) <= 0x19u || str2_current_char >= 0xE0u)
                str2_current_char -= 32;
            if (str1_current_char > str2_current_char)
                break;
            if (str1_current_char < str2_current_char)
                return -1;
            ++str1;
            ++str2;
            if (!str1_current_char || !str2_current_char)
                return 0;
        }
    }
    return 1;

}

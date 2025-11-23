// standard string function implementations as taken from IDA pseudocode
// don't want to break anything by assuming these are identical to standard functionality
// may replace entirely with standard functions later on

#ifndef LEGOINDY_STRINGSSTD_H
#define LEGOINDY_STRINGSSTD_H

// ===================== FUNCTIONS =====================

int __cdecl _strcmpi(char* str1, char* str2);
int __cdecl _strncmp(char* str1, char* str2, int size);
int __cdecl _strlen(char* str);
int __cdecl _strcpy(char* dest, char* src);
char* __cdecl _strcat(char* dest, char* src);

#endif // LEGOINDY_STRINGSSTD_H

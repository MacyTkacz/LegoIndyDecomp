// standard string function implementations as taken from IDA pseudocode
// don't want to break anything by assuming these are identical to standard functionality
// may replace entirely with standard functions later on

#ifndef LEGOINDY_STRINGSSTD_H
#define LEGOINDY_STRINGSSTD_H

// ===================== FUNCTIONS =====================

int _strcmpi(char* str1, char* str2);
int _strncmp(char* str1, char* str2, int size);
int _strlen(char* str);
int _strcpy(char* dest, char* src);
char* _strcat(char* dest, char* src);

#endif // LEGOINDY_STRINGSSTD_H

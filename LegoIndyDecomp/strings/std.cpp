#include "std.h"

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

int __cdecl _strlen(char* str) {
    char* strOffset;
    int i;
    strOffset = str;
    for (i = 0; *strOffset; ++i)
        strOffset++;
    return i;
}

int __cdecl _strncmp(char* str1, char* str2, int size) {

    char* offsetStr1;
    char* offsetStr2;
    char currentCharStr1;
    char currentCharStr2;
    int numOfCharsRemaining;
    int newSize;

    offsetStr1 = str1;
    if (!str1)
        return -1;
    
    offsetStr2 = str2;
    if (!str2)
        return 1;

    numOfCharsRemaining = size;
    switch (size) {
    case 0:
        return 0;
    case -1:
        newSize = _strlen(str1);
LABEL_10:
        numOfCharsRemaining = newSize;
        goto LABEL_11;
    case -2:
        newSize = _strlen(str2);
        goto LABEL_10;
    }

    while (1) {
LABEL_11:

        // convert to uppercase 
        currentCharStr1 = *offsetStr1;
        if ((char)(*offsetStr1 - 97) <= 25 || (char)currentCharStr1 >= 224)
            currentCharStr1 -= 32;
        currentCharStr2 = *offsetStr2;
        if ((char)(*offsetStr2 - 97) <= 25 || (char)currentCharStr2 >= 224)
            currentCharStr2 -= 32;

        if (currentCharStr1 > currentCharStr2)
            return 1;
        if (currentCharStr1 < currentCharStr2)
            break;

        ++offsetStr1;
        ++offsetStr2;
        --numOfCharsRemaining;
        
        if (!currentCharStr1 || !currentCharStr2 || !numOfCharsRemaining)
            return 0;

    }
    return -1;

}

char* __cdecl _strcat(char* dest, char* src) {
    
    char* currentDestOffset;
    char* currentSrcOffset;
    char currentSrcChar;

    // set currentDestOffset to the end of the dest buffer
    for (currentDestOffset = dest; *currentDestOffset; ++currentDestOffset);

    currentSrcOffset = src;
    if (src) {
        do {
            *currentDestOffset = *currentSrcOffset;
            currentSrcChar = *currentSrcOffset;
            ++currentDestOffset;
            ++currentSrcOffset;
        }
        while (currentSrcChar);
    }

    return currentDestOffset;

}

int __cdecl _strcpy(char* dest, char* src) {

    char* currentSrcOffset;
    char* currentDestOffset;
    char currentSrcChar;

    currentSrcOffset = src;
    currentDestOffset = dest;
    if (src) {
        currentSrcChar = *src;
        if (*src) {
            do {
                ++currentSrcOffset;
                *currentDestOffset = currentSrcChar;
                currentSrcChar = *currentSrcOffset;
                ++currentDestOffset;
            }
            while (*currentSrcOffset);
        }
    }
    *currentDestOffset = 0;
    return currentDestOffset - dest;

}

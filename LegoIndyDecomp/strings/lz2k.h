// LZ2K is the file compression algorithm used by LIJ

#pragma once

// ======================= DATA ========================

inline int LZ2K_SizeBitmask = -1;

// ===================== FUNCTIONS =====================

// checks for "LZ2K" header in buffer
// if present, treats next 4 chars as values places
// 1s place, 256s place, 65536s place (2 bytes LE)
int LZ2K_DecodeUncompressedFileSize(char* textBuffer);

// checks for "LZ2K" header in buffer
// if present, treats chars 9-12 as values places
// 1s place, 256s place, 65536s place (2 bytes LE)
// adds 12 for header length
int LZ2K_DecodeCompressedFileSize(char* textBuffer);
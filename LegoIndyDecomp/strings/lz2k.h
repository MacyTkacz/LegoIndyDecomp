// LZ2K is the file compression algorithm used by LIJ

#ifndef LEGOINDY_LZ2K_H
#define LEGOINDY_LZ2K_H

// ======================= DATA ========================

inline int ChunkSizeBitmask = -1;

// ===================== FUNCTIONS =====================

// checks for "LZ2K" header in buffer
// if present, treats next 4 chars as values places
// 1s place, 256s place, 65536s place (2 bytes LE)
int __cdecl LZ2K_DecodeChunkSize(char* textBuffer);

#endif // LEGOINDY_LZ2K_H 
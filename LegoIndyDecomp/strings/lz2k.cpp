#include "lz2k.h"
#include <memory>

int __cdecl LZ2K_DecodeChunkSize(char* textBuffer) {

	char compressionHeader[4];
	char currentChar;

	int currentCharIndex = 0;
	char* currentCharBufferOffset = textBuffer;

	// 1s place, 256s place, 65536s place (2 bytes LE)
	memcpy(compressionHeader, "LZ2K", 4);
	while (1) {
		currentChar = *currentCharBufferOffset++;
		if (currentChar != compressionHeader[currentCharIndex])
			break;
		if (++currentCharIndex >= 4) {
			unsigned int cond1 = *( (unsigned __int16*)currentCharBufferOffset + 1) << 8;
			unsigned int cond2 = ( (unsigned __int8)currentCharBufferOffset[1] | cond1 ) << 8;
			unsigned int cond3 = (unsigned __int8)*currentCharBufferOffset | cond2;
			return ChunkSizeBitmask & cond3;
		}
	}
	return 0;

}

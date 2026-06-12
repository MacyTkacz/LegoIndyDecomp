#include "lz2k.h"
#include <memory.h>

int LZ2K_DecodeUncompressedFileSize(char* textBuffer) {

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
			unsigned int cond1 = *( (unsigned short*)currentCharBufferOffset + 1) << 8;
			unsigned int cond2 = ( (unsigned char)currentCharBufferOffset[1] | cond1 ) << 8;
			unsigned int cond3 = (unsigned char)*currentCharBufferOffset | cond2;
			return LZ2K_SizeBitmask & cond3;
		}
	}
	return 0;

}

int LZ2K_DecodeCompressedFileSize(char* textBuffer) {

	char compressionHeader[4];
	char currentChar;

	int currentCharIndex = 0;
	char* currentCharBufferOffset = textBuffer;

	memcpy(compressionHeader, "LZ2K", 4);

	for (currentCharIndex = 0; currentCharIndex < 4; ++currentCharIndex) {
		currentChar = *currentCharBufferOffset++;
		if (currentChar != compressionHeader[currentCharIndex])
			return 0;
	}

	int cond1 = *((unsigned short*)currentCharBufferOffset + 3) << 8;
	int cond2 = ((unsigned char)currentCharBufferOffset[5] | cond1) << 8;
	int cond3 = (unsigned char)currentCharBufferOffset[4] | cond2;
	int result = LZ2K_SizeBitmask & cond3;

	if (result)
		result += 12;

	return result;
	
}

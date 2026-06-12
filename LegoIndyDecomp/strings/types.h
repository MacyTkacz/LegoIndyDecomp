#ifndef LEGOINDY_STRINGS_TYPES_H
#define LEGOINDY_STRINGS_TYPES_H

// ===================== STRUCTS =====================

struct Hash {
	short nextOnMatch;
	short nextOnNonmatch;
	char* str;
};

#endif // LEGOINDY_STRINGS_TYPES_H
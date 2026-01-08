#ifndef LEGOINDY_UTILS_H
#define LEGOINDY_UTILS_H

#include <Windows.h>

LARGE_INTEGER IntToLargeInteger(int i) {
	LARGE_INTEGER li;
	li.QuadPart = i;
	return li;
}

#endif // LEGOINDY_UTILS_H
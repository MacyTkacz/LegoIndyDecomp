#ifndef LEGOINDY_UTILS_H
#define LEGOINDY_UTILS_H

#include <Windows.h>

// defines a LARGE_INTEGER whose QuadPart is val
#define DEFLARGEINT(name,val) \
LARGE_INTEGER name; \
name.QuadPart = val

template <typename T>
LARGE_INTEGER ToLargeInt(T n) {
	DEFLARGEINT(li, n);
	return li;
}

#endif // LEGOINDY_UTILS_H
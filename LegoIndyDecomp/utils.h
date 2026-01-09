#ifndef LEGOINDY_UTILS_H
#define LEGOINDY_UTILS_H

#include <Windows.h>

template <typename T>
T* GetOnHeap() {
	T* o = (T*)malloc(sizeof(T));
	if (!o)
		return 0;
	memset(o, 0, sizeof(o));
	return o;
}

// allows for tracking of [size] ptrs to heap data
#define HEAP_INIT(size) \
void** heapallocs = (void**)GetOnHeap<void*[size]>(); \
int heapallocs_i = 0

// tracks a heap pointer
#define HEAP_HOOK(ptr) \
heapallocs[heapallocs_i++] = (void*)ptr

// frees all tracked heap pointers
#define HEAP_FREE() \
for (int i=heapallocs_i-1; i>0; --i) {\
	void* ptr = heapallocs[i]; \
	free(ptr); \
} \
free(heapallocs)

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
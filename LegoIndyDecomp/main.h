#ifndef MAIN_H
#define MAIN_H

#include <memory>

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

#endif // MAIN_H

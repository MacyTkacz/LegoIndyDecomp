#pragma once

#ifdef _WIN32
  #include <windows.h>
#else
  #include <cstdint>

  typedef union {
    struct {
      uint32_t LowPart;
      int32_t HighPart;
    };
    int64_t QuadPart;
  } LARGE_INTEGER;

#endif

// defines a LARGE_INTEGER whose QuadPart is val
#define DEFLARGEINT(name,val) \
LARGE_INTEGER name; \
name.QuadPart = val

template <typename T>
LARGE_INTEGER ToLargeInt(T n) {
	DEFLARGEINT(li, n);
	return li;
}
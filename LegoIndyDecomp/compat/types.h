#ifndef LEGOINDY_COMPAT_TYPES_H
#define LEGOINDY_COMPAT_TYPES_H

#ifndef _WIN32

typedef union LARGE_INTEGER {
  struct {
    unsigned long LowPart;
    long HighPart;
  };
  long long QuadPart;
} LARGE_INTEGER;

#endif

#endif LEGOINDY_COMPAT_TYPES_H
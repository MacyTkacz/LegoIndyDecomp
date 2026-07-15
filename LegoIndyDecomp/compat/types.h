#pragma once

#ifndef _WIN32

typedef union LARGE_INTEGER {
  struct {
    unsigned long LowPart;
    long HighPart;
  };
  long long QuadPart = 0;
} LARGE_INTEGER;

#endif
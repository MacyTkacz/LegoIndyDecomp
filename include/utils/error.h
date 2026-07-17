#include <iostream>
#ifndef _WIN32
 #include <cstring>
#endif

namespace ErrorUtils {

void PrintLastErrorStr() {
#ifdef _WIN32
    std::cout << "win32 error code: " << GetLastError() << std::endl;
#else
    std::cout << std::strerror(errno) << std::endl;
#endif
}

}
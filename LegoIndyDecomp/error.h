#ifndef LEGOINDY_ERROR_H
#define LEGOINDY_ERROR_H

#include <stdexcept>

class NotImplemented : public std::logic_error {
public:
    NotImplemented() : std::logic_error("Function not yet implemented") { };
};

#endif // LEGOINDY_ERROR_H
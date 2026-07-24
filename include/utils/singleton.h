#pragma once

#include <memory>

template <typename T> 
class ISingleton {
public:
    static T& Instance() {
        if (!s_pInstance.get())
            s_pInstance = std::make_unique<T>();
        return *s_pInstance;
    }
private:
    inline static std::unique_ptr<T> s_pInstance;
};
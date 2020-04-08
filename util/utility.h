#ifndef UTILITY_H
#define UTILITY_H

#include <memory>

namespace std {

template<typename T, typename... Args>
inline std::unique_ptr<T> make_unique(Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}

inline void dynamic_check(bool flag) {
    if (flag == false) {
        throw std::runtime_error("dynamic_check failed");
    } else {
        // ok
    }
}

#endif

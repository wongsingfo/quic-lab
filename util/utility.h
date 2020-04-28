#ifndef UTILITY_H
#define UTILITY_H

#include <memory>
#include <sstream>
#include <iostream>
#include <cassert>

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

using log_stream = std::ostream;

inline log_stream &log_debug() {
    return std::cerr;
}

#endif

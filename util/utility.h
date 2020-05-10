#ifndef UTILITY_H
#define UTILITY_H

#include <memory>
#include <sstream>
#include <iostream>
#include <cassert>

// https://github.com/amrayn/easyloggingpp
#include "util/easylogging++.h"

using std::unique_ptr;

namespace std {

template<typename T, typename... Args>
inline std::unique_ptr<T> make_unique(Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}

#ifndef htonll

#if __BIG_ENDIAN__
# define htonll(x) (x)
# define ntohll(x) (x)
#else
# define htonll(x) \
      (((uint64_t)htonl((x) & 0xFFFFFFFF) << 32u) | htonl((x) >> 32u))
# define ntohll(x) \
      (((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32u) | ntohl((x) >> 32u))
#endif

#endif

template<typename T> constexpr
static T const& const_max(T const& a, T const& b) {
    return a > b ? a : b;
}

template<typename T> constexpr
static T const& const_min(T const& a, T const& b) {
    return a < b ? a : b;
}

#endif

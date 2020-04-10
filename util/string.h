#ifndef QUIC_BINARY_H
#define QUIC_BINARY_H

#include <cstddef>
#include <cstring>
#include <string>

#include "util/utility.h"

class String {

public:
    using dtype = unsigned char;

    String(const dtype* data, size_t size)
        : String(size) {
        memcpy(data_, data, size);
    }

    String(const char* data, size_t size)
        : String((dtype*)data, size) {}

    explicit String(size_t size)
        : size_(size),
          data_(new dtype[size])
        {}

    ~String() {
        delete[] data_;
    }

    inline void reset() {
        data_ = nullptr;
        size_ = 0;
    }

    inline String copy() {
        return String(data_, size_);
    }

    std::string to_hex() const;

    inline dtype* data() const { return data_; }
    inline size_t size() const { return size_; }

    String clone() const {
        return String(data(), size());
    }

    String(String &&other) noexcept {
        data_ = other.data();
        size_ = other.size();
        other.reset();
    }

    String& operator = (String &&other) noexcept {
        if (this == &other) {
            return *this;
        }

        delete[] data_;

        data_ = other.data();
        size_ = other.size();
        other.reset();
        return *this;
    }

    bool operator == (const String& other) const noexcept;

    // Construct a String from a human-readable representation. The
    // text must end with \0.
    //
    // For example, 0x12ff, 12ff
    static String from_hex(const char* text);

    static String from_text(const char* text);

    inline String sub_string(size_t begin, size_t end) const {
        dynamic_check(begin <= end && end <= size());
        return String(data() + begin, end - begin);
    }

    inline dtype& operator [] (size_t i) const {
        dynamic_check(i <= size());
        return data_[i];
    }

    inline static String empty_string() {
        return String(0);
    }

    // disallow copy and assignment
    String (const String&) = delete;
    String& operator=(const String&) = delete;

    friend std::ostream& operator<<(std::ostream& os, const String& self);

private:

    int size_;
    dtype* data_;

};

#endif

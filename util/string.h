#ifndef QUIC_BINARY_H
#define QUIC_BINARY_H

#include <cstddef>
#include <cstring>
#include <string>

#include "util/utility.h"

class String;

class StringRef {

public:
    using dtype = uint8_t;

    StringRef() = default;

    StringRef(dtype* data, size_t size)
        : data_(data), size_(size)
        {}

    ~StringRef() = default;

    inline dtype* data() const { return data_; }
    inline size_t size() const { return size_; }

    bool operator == (const StringRef& other) const noexcept;

    // the |text| must be a static data
    static StringRef from_text(const char* text);

    inline StringRef sub_string(size_t begin, size_t end) const {
        dynamic_check(begin <= end && end <= size());
        return StringRef(data() + begin, end - begin);
    }

    inline StringRef sub_string(size_t begin) const {
        return sub_string(begin, size());
    }

    inline dtype& operator [] (size_t i) const {
        dynamic_check(i <= size());
        return data_[i];
    }

    inline static StringRef empty_string() {
        return StringRef(nullptr, 0);
    }

    // for testing and debugging
    std::string to_hex() const;

    String clone() const;

    friend std::ostream& operator<<(std::ostream& os, const StringRef& self);

protected:

    size_t size_;
    dtype* data_;

};

class String : public StringRef {

public:

    String(const dtype* data, size_t size)
        : String(size) {
        memcpy(data_, data, size);
    }

    String(const char* data, size_t size)
        : String((dtype*)data, size) {}

    explicit String(size_t size)
        : StringRef(new dtype[size], size)
    {}

    // Construct a String from a human-readable representation. The
    // text must end with \0.
    //
    // For example, 0x12ff, 12ff
    static String from_hex(const char* text);

    static String random(size_t size);

    inline void reset() {
        data_ = nullptr;
        size_ = 0;
    }

    ~String() {
        delete[] data_;
    }

    String(String &&other) noexcept
     : StringRef(other.data(), other.size()) {
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

    // disallow copy and assignment
    String (const String&) = delete;
    String& operator=(const String&) = delete;

};

#endif

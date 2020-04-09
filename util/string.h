#ifndef QUIC_BINARY_H
#define QUIC_BINARY_H

#include <cstddef>
#include <cstring>
#include <string>

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

    // Construct a String from a human-readable representation. The
    // text must end with \0.
    //
    // For example, 0x12ff, 12ff
    static String from_hex(const char* text);

    static String from_text(const char* text);

    inline static String empty_string() {
        return String(0);
    }

    // disallow copy and assignment
    String (const String&) = delete;
    String& operator=(const String&) = delete;

private:

    int size_;
    dtype* data_;

};

#endif

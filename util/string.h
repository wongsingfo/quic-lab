#ifndef QUIC_BINARY_H
#define QUIC_BINARY_H

#include <stddef.h>
#include <string.h>
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

    String(size_t size) 
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

    std::string to_string() const;

    inline dtype* data() const { return data_; }
    inline size_t size() const { return size_; }

    String(String &&other) {
        data_ = other.data();
        size_ = other.size();
        other.reset();
    }

    // Construct a String from a human-readable representation. The
    // text must end with \0.
    //
    // For example, 0x12ff, 12ff
    static String from_text(const char* text);

    // disallow copy and assignment
    String (const String&) = delete; \
    String& operator=(const String&) = delete;

private:

    int size_;
    dtype* data_;

};

#endif

//
// Created by Chengke Wong on 2020/4/8.
//

#ifndef UTIL_STRING_WRITER_H
#define UTIL_STRING_WRITER_H

#include "string_raw.h"

class StringWriter : public StringRef {

private:

    size_t position_;

public:

    StringWriter(dtype* data, size_t size)
        : StringRef(data, size),
          position_(0)
    {}

    StringWriter(StringRef data)
        : StringRef(data),
          position_(0)
    {}

    void write(const dtype* data, size_t length);

    void write(const char* text);

    void write(StringRef s);

    void write_u8(uint8_t value);
    void write_u16(uint16_t value);

    inline size_t remaining() const {
        return size() - position_;
    }

    inline size_t position() const {
        return position_;
    }

    // disallow copy and assignment
    StringWriter (const StringWriter&) = delete;
    StringWriter& operator=(const StringWriter&) = delete;

};


#endif //UTIL_STRING_WRITER_H

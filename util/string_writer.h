//
// Created by Chengke Wong on 2020/4/8.
//

#ifndef UTIL_STRING_WRITER_H
#define UTIL_STRING_WRITER_H

#include "util/string.h"

// The writer owns the |data_| so it is derived from String
// rather than StringRef
class StringWriter : public String {

private:

    size_t position_;

public:

    explicit StringWriter(size_t size)
        : String(size),
          position_(0)
    {}

    void write(const dtype* data, size_t length);

    void write(const char* text);

    void write(StringRef s);

    void write_u8(uint8_t value);
    void write_u16(uint16_t value);

    // disallow copy and assignment
    StringWriter (const StringWriter&) = delete;
    StringWriter& operator=(const StringWriter&) = delete;

};


#endif //UTIL_STRING_WRITER_H

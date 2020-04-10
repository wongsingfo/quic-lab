//
// Created by Chengke Wong on 2020/4/9.
//

#ifndef UTIL_STRING_READER_H
#define UTIL_STRING_READER_H

#include "util/string.h"

class StringReader : public String {

private:

    size_t position_;

public:

    StringReader(const dtype* data, size_t size)
        : String(data, size),
          position_(0)
    {}

    explicit StringReader(const String &s)
        : StringReader(s.data(), s.size())
    {}

    void read(const dtype* data, size_t length);

    inline size_t position() const { return position_; }

    uint64_t read_with_variant_length();

    inline void read(String &dest, size_t length) {
        read(dest.data(), length);
    }

    inline void read(String &dest) {
        read(dest.data(), dest.size());
    }

    uint8_t peek_u8();
    uint8_t read_u8();

    uint16_t read_u16();
    uint32_t read_u32();
    uint64_t read_u64();

    // disallow copy and assignment
    StringReader (const StringReader&) = delete;
    StringReader& operator=(const StringReader&) = delete;

};



#endif //UTIL_STRING_READER_H

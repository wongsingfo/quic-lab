//
// Created by Chengke Wong on 2020/4/9.
//

#ifndef UTIL_STRING_READER_H
#define UTIL_STRING_READER_H

#include "string_raw.h"

class StringReader : public StringRef {

private:

    size_t position_;

public:

    StringReader(dtype* data, size_t size)
        : StringRef(data, size),
          position_(0)
    {}

    explicit StringReader(const String &s)
        : StringReader(s.data(), s.size())
    {}

    void read(const dtype* data, size_t length);

    inline size_t position() const { return position_; }

    inline bool empty() const {
        return position_ == size_;
    }

    uint64_t read_with_variant_length();

    inline void read(String &dest, size_t length) {
        read(dest.data(), length);
    }

    inline void read(String &dest) {
        read(dest.data(), dest.size());
    }

    void skip(size_t length);

    inline size_t remaining() const {
        return size() - position_;
    }

    inline dtype* peek_data() const {
        return this->data() + position_;
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

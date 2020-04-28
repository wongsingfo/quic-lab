//
// Created by Chengke Wong on 2020/4/9.
//

#include "util/string_reader.h"

#include <arpa/inet.h>

void StringReader::read(const String::dtype *data, size_t length) {
    if (position_ + length > size()) {
        throw std::overflow_error("StringReader::read");
    }

    dtype* p = this->data() + position_;

    memcpy((void *) data, p, length);

    position_ += length;
}

uint8_t StringReader::peek_u8() {
    if (position_ == size()) {
        throw std::overflow_error("peek_u8");
    }

    dtype* p = this->data() + position_;
    return *p;
}

uint8_t StringReader::read_u8() {
    uint8_t result = peek_u8();
    position_ += sizeof(uint8_t);
    return result;
}

uint16_t StringReader::read_u16() {
    uint16_t value;
    read((dtype*) &value, sizeof value);
    return ntohs(value);
}

uint32_t StringReader::read_u32() {
    uint32_t value;
    read((dtype*) &value, sizeof value);
    return ntohl(value);
}

uint64_t StringReader::read_u64() {
    uint64_t value;
    read((dtype*) &value, sizeof value);
    return ntohll(value);
}

uint64_t StringReader::read_with_variant_length() {
/*
 * 2Bit	Length	Usable Bits	Range
 * 00	1	6	0-63
 * 01	2	14	0-16383
 * 10	4	30	0-1073741823
 * 11	8	62	0-4611686018427387903
 */

    uint8_t i = (peek_u8() & 0xc0) >> 6;
    switch (i) {
        case 0:
            return read_u8() & 0x3f;
        case 1:
            return read_u16() & 0x3fff;
        case 2:
            return read_u32() & 0x3fffffff;
        case 3:
            return read_u64() & 0x3fffffffffffffff;
        default:
            throw std::runtime_error("this should never happen");
    }
}

void StringReader::skip(size_t length) {
    position_ += length;
    if (position_ > size()) {
        throw std::overflow_error("skip");
    }
}

//
// Created by Chengke Wong on 2020/4/8.
//

#include "string_writer.h"

#include <arpa/inet.h>

void StringWriter::write(const String::dtype *data, size_t length) {
    if (position_ + length > size()) {
        throw std::overflow_error("StringWriter::write");
    }

    dtype* p = this->data() + position_;

    memcpy(p, data, length);

    position_ += length;
}

void StringWriter::write(const char *text) {
    write((dtype*) text, strlen(text));
}

void StringWriter::write(const StringRef s) {
    write(s.data(), s.size());
}

void StringWriter::write_u8(uint8_t value) {
    return write((dtype*) &value, sizeof(value));
}

void StringWriter::write_u16(uint16_t value) {
    value = htons(value);
    return write((dtype*) &value, sizeof(value));
}


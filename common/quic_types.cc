//
// Created by Chengke Wong on 2020/4/9.
//

#include "common/quic_types.h"

Cid Cid::from_reader(StringReader &reader)  {
    uint8_t length = reader.read_u8();
    if (length > 20) {
        throw error_discard_packet("the CID is longer than 20 bytes");
    }

    Cid result(length);
    reader.read(result);

    return result;
}

Token Token::from_reader(StringReader &reader) {
    uint64_t length = reader.read_with_variant_length();

    Token result(length);
    reader.read(result);

    return result;
}

PacketNumber::dtype PacketNumber::from_string(StringRef s, size_t length) {
    if (s.size() < length) {
        throw std::overflow_error("string is too short to get a packet number");
    }

    switch (length) {
        case 1:
            return s[0];
        case 2:
            return (s[0] << 8) | s[1];
        case 3:
            return (s[0] << 16) | (s[1] << 8) | s[2];
        case 4:
            return (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3];
        default:
            throw std::invalid_argument(
                "the packet number length should no more than 32 bit");
    }
}

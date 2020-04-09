//
// Created by Chengke Wong on 2020/4/9.
//

#include "types.h"

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

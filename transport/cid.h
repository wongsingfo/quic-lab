//
// Created by Chengke Wong on 2020/4/9.
//

#ifndef TRANSPORT_CID_H
#define TRANSPORT_CID_H

#include "util/string.h"
#include "util/utility.h"
#include "util/string_reader.h"

#include "transport/exception.h"

/*
 * Connection ID (CID) encoding:
 *    1. a byte indicating the length of the CID. In current QUIC version,
 *    this value MUST NOT exceed 20.
 *
 *    2. the CID [0..20] bytes in length (i.e. [0..160] bits)
 */

class Cid : public String {

public:

    template<typename... Args>
    explicit Cid(Args&&... args)
        : String(std::forward<Args>(args)...)
        {}

    static Cid from_reader(StringReader &reader) {
        uint8_t length = reader.read_u8();
        if (length > 20) {
            throw error_discard_packet("the CID is longer than 20 bytes");
        }

        Cid result(length);
        reader.read(result, length);

        return result;
    }

};

#endif //TRANSPORT_CID_H

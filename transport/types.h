//
// Created by Chengke Wong on 2020/4/9.
//

#ifndef TRANSPORT_TYPES_H
#define TRANSPORT_TYPES_H

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

    static Cid from_reader(StringReader &reader);
};

class Token : public String {

public:
    template<typename... Args>
    explicit Token(Args &&... args)
        : String(std::forward<Args>(args)...)
        {}

    static Token from_reader(StringReader &reader);
};


#endif //TRANSPORT_TYPES_H

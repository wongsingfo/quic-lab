//
// Created by Chengke Wong on 2020/4/9.
//

#ifndef TRANSPORT_TYPES_H
#define TRANSPORT_TYPES_H

#include "util/string.h"
#include "util/utility.h"
#include "util/string_reader.h"

#include "transport/exception.h"

enum class Endpoint : bool {
    Client,
    Server,
};

enum StreamDirection : bool {
    Bidirectional,
    Unidirectional,
};

enum ReadWriteMode : bool {
    ReadMode,
    WriteMode,
};

class StreamId {

public: 
    StreamId() = default;
    ~StreamId() = default;

    StreamId(uint64_t id) 
        : id_(id) {}

private:
    uint64_t id_;

};

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

class PacketNumber {

public:
    using dtype = uint64_t;

    PacketNumber(dtype value)
    : value(value) {}

    PacketNumber() = default;

    dtype value;

    static PacketNumber::dtype from_string(StringRef s, size_t length);

    inline bool operator< (const PacketNumber &o) const {
        return value < o.value;
    }

    inline bool operator> (const PacketNumber &o) const {
        return value > o.value;
    }

};

using Version = uint32_t;

// packet number spaces
constexpr size_t kNumOfPNSpaces = 3;
enum class PNSpace {
    Intial = 0,
    Handshake = 1,
    Application = 2,
};

#endif //TRANSPORT_TYPES_H

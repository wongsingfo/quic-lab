//
// Created by Chengke Wong on 2020/4/9.
//

#ifndef TRANSPORT_PACKET_HEADER_H
#define TRANSPORT_PACKET_HEADER_H

#include <cstdint>
#include <vector>

#include "util/optional.h"
#include "transport/cid.h"

// TODO

enum class PacketType {
    // Long Header Packet Types
    Initial = 0,
    ZeroRTT = 1,
    Handshake = 2,
    Retry = 3,
    // Version Negotiation
    VersionNegotiation = 4,
    // Short Header Packet Types
    OneRTT = 5,
};

using Version = uint32_t;
using PacketNumber = uint64_t;

using std::experimental::optional;

class PacketHeader {

private:
    PacketType type_;
    Version version_;
    Cid scid_;
    Cid dcid_;
    PacketNumber number;

    optional< String > Token;
    optional< std::vector<Version> > versions_;

public:
    PacketHeader(Cid &&scid, Cid &&dcid)
        : scid_(std::move(scid)),
          dcid_(std::move(dcid)) {}

    static PacketHeader from_reader(StringReader &reader);

    static PacketHeader long_packet_from_reader(StringReader &reader);
};


#endif //TRANSPORT_PACKET_HEADER_H

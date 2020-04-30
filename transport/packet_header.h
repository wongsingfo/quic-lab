//
// Created by Chengke Wong on 2020/4/9.
//

#ifndef TRANSPORT_PACKET_HEADER_H
#define TRANSPORT_PACKET_HEADER_H

#include <cstdint>
#include <vector>

#include "util/optional.h"
#include "common/types.h"

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

using std::experimental::optional;

struct PacketHeader {

    PacketType type;
    Version version;
    Cid scid;
    Cid dcid;

    // the length includes both the Packet Number and Payload fields
    size_t length;

    // starting from the first byte up to but not including packet number
    size_t header_length;

    // only present in VersionNegotiation
    optional< std::vector<Version> > versions_;

    // [encrypted] only in long packet header
    size_t pkt_number_len;

    // [encrypted]
    PacketNumber pkt_number;

    // only present in Initial and Retry
    optional<String> token;

    PacketHeader(Cid &&scid, Cid &&dcid)
        : scid(std::move(scid)),
          dcid(std::move(dcid)) {}

    /* The header protection algorithm uses both the header protection key
     * and a sample of the ciphertext from the packet Payload field. */

    void decrypt(StringRef hp, StringRef packet);

    static PacketHeader from_reader(StringReader &reader);

    static PacketHeader long_packet_from_reader(StringReader &reader);

    inline size_t payload_offset() const {
        return pkt_number_len + header_length;
    }
};


#endif //TRANSPORT_PACKET_HEADER_H

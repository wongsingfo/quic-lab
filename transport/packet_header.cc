//
// Created by Chengke Wong on 2020/4/9.
//

#include "transport/packet_header.h"
#include "crypto/aead.h"

static inline bool is_long_packet(uint8_t first_byte) {
    // The most significant bit (0x80) of byte 0 (the first byte) is set to 1 for long headers
    bool is_long = first_byte & 0x80;

    if ((first_byte & 0x40) == 0) {
        throw error_discard_packet("Fixed Bit must be one");
    }

    return is_long;
}

static inline PacketType long_packet_type(uint8_t first_byte) {
    uint8_t type = first_byte & 0x30;
    type >>= 4u;
    return static_cast<PacketType>(type);
}

PacketHeader PacketHeader::from_reader(StringReader &reader) {
    uint8_t first_byte = reader.peek_u8();

    if (is_long_packet(first_byte)) {
        return long_packet_from_reader(reader);
    } else {

    }
}

PacketHeader PacketHeader::long_packet_from_reader(StringReader &reader) {
// https://quicwg.org/base-drafts/draft-ietf-quic-transport.html#name-long-header-packets

// The header form bit, connection ID lengths byte, Destination and Source
// Connection ID fields, and Version fields of a long header packet are
// version-independent.

    uint8_t first_byte = reader.read_u8();
    PacketType type = long_packet_type(first_byte);
    // The other fields in the first byte are version-specific.

    Version version = reader.read_u32();
    Cid dcid = Cid::from_reader(reader);
    Cid scid = Cid::from_reader(reader);

    PacketHeader result(std::move(scid), std::move(dcid));
    result.type = type;
    result.version = version;

    if (type == PacketType::Initial) {
        result.token = Token::from_reader(reader);
        result.length = reader.read_with_variant_length();
        result.header_length = reader.position();
    }

    return result;
}

void PacketHeader::decrypt(StringRef hp, StringRef packet) {
    // In sampling the packet ciphertext, the Packet Number field is assumed to
    // be 4 bytes long (its maximum possible encoded length).
    StringRef sample = packet.sub_string(header_length + 4,
                                         header_length + 4 + 16);

    String mask = crypto::aes_128_ecb_encrypt(hp, sample);
    uint8_t first_byte = packet[0];
    if (is_long_packet(first_byte)) {
        first_byte ^= mask[0] & 0x0f;
    } else {
        first_byte ^= mask[0] & 0x1f;
    }
    packet[0] = first_byte;

    int pn_length = (first_byte & 0x03) + 1;
    int pn_offset = header_length;
    for (int i = 0; i < pn_length; i++) {
        packet[pn_offset + i] ^= mask[1 + i];
    }
}


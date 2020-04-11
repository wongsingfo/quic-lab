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

/* Packet Header Protection
 *   This process does not apply to Retry or Version Negotiation packets
 * https://quicwg.org/base-drafts/draft-ietf-quic-tls.html#name-header-protection
 *
 * hp_key = HKDF_Expand(secret, "quic hp", _, 16)
 *
 * Prior to TLS selecting a ciphersuite, AES header protection is used,
 * matching the AEAD_AES_128_GCM packet protection.
 *
 * sample = starting from the payload (the Packet Number field is
            assumed to be 4 bytes long) up to 16 bytes
 * mask   = AES-ECB(hp_key, sample)
 *   - AEAD_AES_128_GCM and AEAD_AES_128_CCM use 128-bit AES in electronic code-book (ECB) mode.
 *   - AEAD_AES_256_GCM uses 256-bit AES in ECB mode.
 *
 * counter= sample[0..3]
 * nounce = sample[4..15]
 * mask   = ChaCha20(hp_key, counter, nonce, {0,0,0,0,0})
 */
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

    pkt_number_len = pn_length;

    int pn_offset = header_length;
    for (int i = 0; i < pn_length; i++) {
        packet[pn_offset + i] ^= mask[1 + i];
    }

    // Two bits (those with a mask of 0x0c) of byte 0 are reserved across
    // multiple packet types. These bits are protected using header protection
    // (see Section 5.4 of [QUIC-TLS]). The value included prior to protection
    // MUST be set to 0.

    if (first_byte & 0x0c) {
        throw error_protocol_violation("The Reserved Bits are not zero");
    }

    // Now the header decryption is done; we next need to recover
    // the missing data.
    pkt_number.value = PacketNumber::from_string(
        packet.sub_string(header_length), pn_length);

}


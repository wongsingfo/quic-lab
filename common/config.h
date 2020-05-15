//
// Created by Chengke Wong on 2020/4/29.
//

#ifndef QUIC_CONFIG_H
#define QUIC_CONFIG_H

#include <cstdint>

#include "util/instant.h"

// Timer granularity. This is a system-dependent value
constexpr Duration kTimerGranularity = Duration::from_milliseconds(1);

// https://quicwg.org/base-drafts/draft-ietf-quic-transport.html#name-path-maximum-transmission-u
// The PMTU is the maximum size of the entire IP packet including the IP header, UDP header, and UDP payload.
// The UDP payload includes the QUIC packet header, protected payload, and any authentication fields.
//
// QUIC depends on a PMTU of at least 1280 bytes. This is the IPv6 minimum size [RFC8200] and is also supported
// by most modern IPv4 networks.
//
// Assuming the minimum IP header size, this results in a QUIC maximum packet size of 1232 bytes for IPv6
// and 1252 bytes for IPv4.
constexpr size_t kMaxDatagramSize = 1280;

struct QuicConfig {
    // sending buffer size
    size_t tx_buffer_size = 64 * 1024; // 64 KB
};

extern QuicConfig default_quic_config;

#endif

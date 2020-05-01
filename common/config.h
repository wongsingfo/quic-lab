//
// Created by Chengke Wong on 2020/4/29.
//

#ifndef QUIC_CONFIG_H
#define QUIC_CONFIG_H

#include <cstdint>

#include "util/time.h"

// Timer granularity. This is a system-dependent value
constexpr Duration kTimerGranularity = Duration::from_milliseconds(1);

struct QuicConfig {
    // sending buffer size
    size_t tx_buffer_size = 64 * 1024; // 64 KB
};

extern QuicConfig default_quic_config;

#endif

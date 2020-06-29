//
// Created by Chengke Wong on 2020/4/24.
//

#ifndef TRANSPORT_TRANSPORT_PARAMS_H
#define TRANSPORT_TRANSPORT_PARAMS_H

#include "transport/types.h"
#include "util/optional.h"

using std::experimental::optional;

struct TransportParams {
    // 0x00 - 0x04
    optional<Cid> original_connection_id;
    uint64_t max_idle_timeout;
    optional<Token> stateless_reset_token;
    uint64_t max_udp_payload_size = 65527;
    uint64_t initial_max_data;

    // 0x05 - 0x09
    uint64_t initial_max_stream_data_bidi_local;
    uint64_t initial_max_stream_data_bidi_remote;
    uint64_t initial_max_stream_data_uni;
    uint64_t initial_max_streams_bidi;
    uint64_t initial_max_streams_uni;

    // 0x0a - 0x0xe
    uint64_t ack_delay_exponent = 3;
    uint64_t max_ack_delay = 25;
    bool disable_active_migration;
//  IpAddress  uint64_t preferred_address;
    uint64_t active_connection_id_limit;
};


#endif //TRANSPORT_TRANSPORT_PARAMS_H

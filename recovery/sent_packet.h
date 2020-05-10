//
// Created by Chengke Wong on 2020/4/30.
//

#ifndef SENT_PACKET_H
#define SENT_PACKET_H

#include "util/time.h"
#include "recovery/recovery_token.h"

// https://quicwg.org/base-drafts/draft-ietf-quic-recovery.html#name-sent-packet-fields
// record the information about a packet that was sent
struct SentPacket {

    RecoverTokens frames;

    Instant time_sent;

    // Instant time_declared_lost;

    // The number of bytes sent in the packet, not including UDP or 
    // IP overhead, but including QUIC framing overhead.
    size_t sent_bytes;

    // Packets that contain ack-eliciting frames elicit an ACK from 
    // the receiver within the maximum ack delay and are called 
    // ack-eliciting packets. (All frames other than ACK, PADDING, 
    // and CONNECTION_CLOSE are considered ack-eliciting.)
    bool ack_eliciting;

    bool pto;

    // Packets are considered in-flight when they are ack-eliciting 
    // or contain a PADDING frame, and they have been sent but are 
    // not acknowledged, declared lost, or abandoned along with old
    // keys.
    bool in_flight;

};

#endif

//
// Created by Chengke Wong on 2020/4/18.
//

#ifndef TRANSPORT_FRAME_H
#define TRANSPORT_FRAME_H

#include <vector>

#include "util/optional.h"
#include "transport/types.h"

enum class FrameType {
    PADDING = 0,
    PING = 1,
    ACK = 2,
    ACK_ECN = 3,
    RESET = 4,
    STOP_SENDING = 5,
    CRYPTO = 6,
};

using std::experimental::optional;

struct PaddingFrame {
    size_t size;

    static PaddingFrame from_reader(StringReader &reader);
};

struct PingFrame {
    /* empty */

    static PingFrame from_reader(StringReader &reader) { return {}; }
};

struct AckRange {
    uint64_t start;
    uint64_t length;

    AckRange(int64_t start, int64_t length);
};

struct AckFrame {
    bool is_ECN;

    uint64_t largest_ack;
    uint64_t ack_delay;

    // vector of (start, length) pairs
    std::vector<AckRange> ranges;

    uint64_t ECT0_count;
    uint64_t ECT1_count;
    uint64_t ECN_CE_count;

    static AckFrame *from_reader(StringReader &reader, bool ecn);
};

struct ResetFrame {
    StreamId stream_id;
    QuicError error;
    uint64_t final_size;

    static ResetFrame from_reader(StringReader &reader);
};

struct StopSendingFrame {
    StreamId stream_id;
    QuicError error;

    static StopSendingFrame from_reader(StringReader &reader);
};

// The stream does not have an explicit end, so CRYPTO frames do not have
// a FIN bit.
struct CryptoFrame {
    uint64_t offset;
    StringRef data;

    static CryptoFrame from_reader(StringReader &reader);
};

// Version Negotiation, Stateless Reset, and Retry packets do not
// contain frames.

struct Frame;
using Frames = std::vector< Frame >;

struct Frame {
    FrameType type;

    union {
        PaddingFrame padding;
        PingFrame ping;
        AckFrame *ack;
        ResetFrame reset;
        CryptoFrame crypto;
        StopSendingFrame stop_sending;
    };

    void delete_frame();

    static Frames from_reader(StringReader &reader);
};


#endif //TRANSPORT_FRAME_H

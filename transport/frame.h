//
// Created by Chengke Wong on 2020/4/18.
//

#ifndef TRANSPORT_FRAME_H
#define TRANSPORT_FRAME_H

#include <vector>

#include "util/optional.h"
#include "transport/types.h"

constexpr int kNumOfFrameTypes = 21;

using FrameType = uint64_t;

constexpr FrameType FRAME_TYPE_PADDING = 0x0;
constexpr FrameType FRAME_TYPE_PING = 0x1;

// we store these two types into the same structure
constexpr FrameType FRAME_TYPE_ACK = 0x2;
constexpr FrameType FRAME_TYPE_ACK_ECN = 0x3;

constexpr FrameType FRAME_TYPE_RST_STREAM = 0x4;
constexpr FrameType FRAME_TYPE_STOP_SENDING = 0x5;
constexpr FrameType FRAME_TYPE_CRYPTO = 0x6;
constexpr FrameType FRAME_TYPE_NEW_TOKEN = 0x7;

// The STREAM frame takes the form 0b00001XXX (or the set of values 
// from 0x08 to 0x0f). The value of the three low-order bits of the 
// frame type determines the fields that are present in the frame.
constexpr FrameType FRAME_TYPE_STREAM = 0x8;

constexpr FrameType FRAME_TYPE_STREAM_MAX = 0xf;
constexpr FrameType FRAME_TYPE_MAX_DATA = 0x10;
constexpr FrameType FRAME_TYPE_MAX_STREAM_DATA = 0x11;
constexpr FrameType FRAME_TYPE_MAX_STREAMS_BIDI = 0x12;
constexpr FrameType FRAME_TYPE_MAX_STREAMS_UNIDI = 0x13;
constexpr FrameType FRAME_TYPE_DATA_BLOCKED = 0x14;
constexpr FrameType FRAME_TYPE_STREAM_DATA_BLOCKED = 0x15;
constexpr FrameType FRAME_TYPE_STREAMS_BLOCKED_BIDI = 0x16;
constexpr FrameType FRAME_TYPE_STREAMS_BLOCKED_UNIDI = 0x17;
constexpr FrameType FRAME_TYPE_NEW_CONNECTION_ID = 0x18;
constexpr FrameType FRAME_TYPE_RETIRE_CONNECTION_ID = 0x19;
constexpr FrameType FRAME_TYPE_PATH_CHALLENGE = 0x1a;
constexpr FrameType FRAME_TYPE_PATH_RESPONSE = 0x1b;
constexpr FrameType FRAME_TYPE_CONNECTION_CLOSE_TRANSPORT = 0x1c;
constexpr FrameType FRAME_TYPE_CONNECTION_CLOSE_APPLICATION = 0x1d;
constexpr FrameType FRAME_TYPE_HANDSHAKE_DONE = 0x1e;

constexpr FrameType STREAM_FRAME_BIT_FIN = 0x01;
constexpr FrameType STREAM_FRAME_BIT_LEN = 0x02;
constexpr FrameType STREAM_FRAME_BIT_OFF = 0x04;

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

struct NewTokenFrame {
    Token token;

    static NewTokenFrame *from_reader(StringReader &reader);
};

struct StreamFrame {
    StreamId stream_id;
    uint64_t offset;
    StringRef data;
    bool fin;

    static StreamFrame from_reader(StringReader &reader, FrameType type_id);
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
        StopSendingFrame stop_sending;
        CryptoFrame crypto;
        NewTokenFrame *new_token;
        StreamFrame stream;
    };

    void delete_frame();

    static Frames from_reader(StringReader &reader);
};


#endif //TRANSPORT_FRAME_H

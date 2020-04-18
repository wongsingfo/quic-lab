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
    FRAME_TYPE_CRYPTO = 6,
};

using std::experimental::optional;

// we omit padding because it has no semantic value.
// struct PaddingFrame {};

struct PingFrame{};

struct AckFrame {
};

// The stream does not have an explicit end, so CRYPTO frames do not have
// a FIN bit.
struct CryptoFrame {
    Offset offset;
    StringRef data;
};

// Version Negotiation, Stateless Reset, and Retry packets do not
// contain frames.

struct Frame;
using Frames = std::vector< Frame >;

struct Frame {
    FrameType type;

    union {
        PingFrame ping;
        CryptoFrame crypto;
    };

    static Frames from_reader(StringReader &reader);
};


#endif //TRANSPORT_FRAME_H

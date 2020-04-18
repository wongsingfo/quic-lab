//
// Created by Chengke Wong on 2020/4/18.
//

#include "transport/frame.h"

Frames Frame::from_reader(StringReader &reader) {
    Offset offset;
    size_t length;

    Frames result;

    while (! reader.empty()) {
        FrameType type = (FrameType) reader.read_with_variant_length();
        Frame frame;
        frame.type = type;

        switch (type) {
            case FrameType::PADDING:
                break;
            case FrameType::PING:
                frame.ping = PingFrame{};
                break;
            case FrameType::ACK:
                break;
            case FrameType::FRAME_TYPE_CRYPTO:
                offset = reader.read_with_variant_length();
                length = reader.read_with_variant_length();
                frame.crypto = CryptoFrame{offset, {reader.peek_data(), length}};
                reader.skip(length);
                break;
        }

        result.push_back(frame);
    }
    return result;
}

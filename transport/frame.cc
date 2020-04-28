//
// Created by Chengke Wong on 2020/4/18.
//

#include "transport/frame.h"

Frames Frame::from_reader(StringReader &reader) {

    Frames result;

    while (!reader.empty()) {
        Frame frame;
        uint64_t type_id = reader.read_with_variant_length();
        if (type_id >= kNumOfFrameTypes) {
            throw quic_error(QuicError::FRAME_ENCODING_ERROR);
        }
        constexpr uint64_t ack_ecn = 3;

        FrameType type = (type_id == ack_ecn ? 
            FrameType::ACK : (FrameType) type_id);
        frame.type = type;

        switch (type) {
            case FrameType::PADDING:
                frame.padding = PaddingFrame::from_reader(reader);
                break;
            case FrameType::PING:
                frame.ping = PingFrame::from_reader(reader);
                break;
            case FrameType::ACK:
                frame.ack = AckFrame::from_reader(reader, ack_ecn == type_id);
                break;
            case FrameType::RESET:
                frame.reset = ResetFrame::from_reader(reader);
                break;
            case FrameType::STOP_SENDING:
                frame.stop_sending = StopSendingFrame::from_reader(reader);
                break;
            case FrameType::CRYPTO:
                frame.crypto = CryptoFrame::from_reader(reader);
                break;
            case FrameType::NEW_TOKEN:
                frame.token = NewTokenFrame::from_reader(reader);
                break;
        }

        result.push_back(frame);
    }

    return result;
}

void Frame::delete_frame() {
    switch (type) {
        case FrameType::PADDING:
        case FrameType::PING:
            return;
        
        case FrameType::ACK:
            delete ack;
            return;

        case FrameType::CRYPTO:
        case FrameType::RESET:
        case FrameType::STOP_SENDING:
            return;

        case FrameType::NEW_TOKEN:
            delete token;
    }
}

PaddingFrame PaddingFrame::from_reader(StringReader &reader) {
    size_t size = 1;

    while (!reader.empty() && reader.peek_u8() == 0) {
        reader.skip(1);
        size += 1;
    }

    return PaddingFrame{size};
}

CryptoFrame CryptoFrame::from_reader(StringReader &reader) {
    uint64_t offset;
    size_t length;

    offset = reader.read_with_variant_length();
    length = reader.read_with_variant_length();
    StringRef data = {reader.peek_data(), length};

    reader.skip(length);

    return CryptoFrame{offset, data};
}

AckRange::AckRange(int64_t start, int64_t length) 
    : start(start), length(length) {
    if (start < 0 || length < 0) {
        throw quic_error(QuicError::FRAME_ENCODING_ERROR);
    }
}

AckFrame *AckFrame::from_reader(StringReader &reader, bool ecn) {
    // The packet number is an integer in the range 0 to 2^62-1. 
    uint64_t largest_ack = reader.read_with_variant_length();
    uint64_t ack_delay = reader.read_with_variant_length();
    size_t range_cnt = reader.read_with_variant_length();
    uint64_t first_ack_range = reader.read_with_variant_length();

    std::vector<AckRange> ranges;
    ranges.emplace_back(largest_ack - first_ack_range, first_ack_range + 1);
    for (uint i = 0; i < range_cnt; i++) {
        uint64_t gap = reader.read_with_variant_length();
        uint64_t ack_range = reader.read_with_variant_length();
        uint64_t largest_ack = ranges.back().start - gap - 2;
        ranges.emplace_back(largest_ack - ack_range, ack_range + 1);
    }

    AckFrame *frame = new AckFrame {
        .is_ECN = ecn,
        .largest_ack = largest_ack,
        .ack_delay = ack_delay,
        .ranges = std::move(ranges),
    };

    if (ecn) {
        frame->ECT0_count = reader.read_with_variant_length();
        frame->ECT1_count = reader.read_with_variant_length();
        frame->ECN_CE_count = reader.read_with_variant_length();
    }

    return frame;
}

ResetFrame ResetFrame::from_reader(StringReader &reader) {
    uint64_t stream_id = reader.read_with_variant_length();
    uint64_t error = reader.read_with_variant_length();
    // TODO: check if |error| in range of QuicError
    uint64_t final_size = reader.read_with_variant_length();
    return ResetFrame {
        .stream_id = StreamId(stream_id),
        .error = (QuicError) error,
        .final_size = final_size,
    };
}

StopSendingFrame StopSendingFrame::from_reader(StringReader &reader) {
    uint64_t stream_id = reader.read_with_variant_length();
    uint64_t error = reader.read_with_variant_length();
    // TODO: check if |error| in range of QuicError
    return StopSendingFrame {
        .stream_id = StreamId(stream_id),
        .error = (QuicError) error,
    };
}

NewTokenFrame *NewTokenFrame::from_reader(StringReader &reader) {
    uint64_t length = reader.read_with_variant_length();
    Token token = Token(reader.peek_data(), length);
    reader.skip(length);
    return new NewTokenFrame {
        .token = std::move(token),
    };
}




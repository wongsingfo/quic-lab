//
// Created by Chengke Wong on 2020/4/18.
//

#include "transport/frame.h"

Frames Frame::from_reader(StringReader &reader) {

    Frames result;

    while (!reader.empty()) {
        Frame frame;
        FrameType type_id = reader.read_with_variant_length();

        frame.type = type_id;

        switch (type_id) {
            case FRAME_TYPE_PADDING:
                frame.padding = PaddingFrame::from_reader(reader);
                break;
            case FRAME_TYPE_PING:
                frame.ping = PingFrame::from_reader(reader);
                break;
            case FRAME_TYPE_ACK:
                frame.ack = AckFrame::from_reader(reader, false);
                break;
            case FRAME_TYPE_ACK_ECN:
                frame.type = FRAME_TYPE_ACK;
                frame.ack = AckFrame::from_reader(reader, true);
                break;
            case FRAME_TYPE_RST_STREAM:
                frame.reset = ResetFrame::from_reader(reader);
                break;
            case FRAME_TYPE_STOP_SENDING:
                frame.stop_sending = StopSendingFrame::from_reader(reader);
                break;
            case FRAME_TYPE_CRYPTO:
                frame.crypto = CryptoFrame::from_reader(reader);
                break;
            case FRAME_TYPE_NEW_TOKEN:
                frame.new_token = NewTokenFrame::from_reader(reader);
                break;
            case FRAME_TYPE_STREAM ... FRAME_TYPE_STREAM_MAX:
                frame.type = FRAME_TYPE_STREAM;
                frame.stream = StreamFrame::from_reader(reader, type_id);
                break;

            case FRAME_TYPE_MAX_DATA:
            case FRAME_TYPE_MAX_STREAM_DATA:

            case FRAME_TYPE_DATA_BLOCKED:
            case FRAME_TYPE_STREAM_DATA_BLOCKED:
                frame.type = FRAME_TYPE_MAX_DATA;
                frame.max_data = MaxDataFrame::from_reader(reader, type_id);

            case FRAME_TYPE_MAX_STREAMS_BIDI:
            case FRAME_TYPE_MAX_STREAMS_UNIDI:

            case FRAME_TYPE_STREAMS_BLOCKED_BIDI:
            case FRAME_TYPE_STREAMS_BLOCKED_UNIDI:
                frame.type = FRAME_TYPE_MAX_STREAMS_BIDI;
                frame.max_stream = MaxStreamFrame::from_reader(reader, type_id);
                break;

            case FRAME_TYPE_HANDSHAKE_DONE:
                frame.handshake_done = HandshakeDoneFrame::from_reader(reader);
                break;
            default:
                throw quic_error(QuicError::FRAME_ENCODING_ERROR);
        }

        result.push_back(frame);
    }

    return result;
}

void Frame::delete_frame() {
    switch (type) {
        case FRAME_TYPE_ACK:
            delete ack;
        case FRAME_TYPE_NEW_TOKEN:
            delete new_token;
        case FRAME_TYPE_STREAM:
            delete stream;
        default:
            return;
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

StreamFrame *StreamFrame::from_reader(StringReader &reader, FrameType type_id) {
    StreamId stream_id = reader.read_with_variant_length();
    uint64_t offset = 0;
    if (type_id & STREAM_FRAME_BIT_OFF) {
        offset = reader.read_with_variant_length();
    }
    uint64_t length;
    if (type_id & STREAM_FRAME_BIT_LEN) {
        length = reader.read_with_variant_length();
    } else {
        // the rest of the payload
        length = reader.remaining();
    }
    StringRef data = {reader.peek_data(), length};
    reader.skip(length);
    return new StreamFrame {
        .stream_id = stream_id,
        .offset = offset,
        .data = data,
        .fin = static_cast<bool>(type_id & STREAM_FRAME_BIT_FIN),
    };
}

MaxDataFrame MaxDataFrame::from_reader(StringReader &reader, FrameType type_id) {
    bool is_connection_level = (type_id & 0x1) == 0;
    bool is_block = (type_id & 0x4) == 1;
    StreamId stream_id;
    if (!is_connection_level) {
        stream_id = reader.read_with_variant_length();
    }

    uint64_t max_data = reader.read_with_variant_length();
    return MaxDataFrame {
        .is_connection_level = is_connection_level,
        .is_block = is_block,
        .stream_id = stream_id,
        .max_data = max_data,
    };
}

MaxStreamFrame MaxStreamFrame::from_reader(StringReader &reader, FrameType type_id) {
    bool is_bidirectional = (type_id & 0x1) == 0;
    bool is_block = (type_id & 0x4) == 1;
    uint64_t max_stream = reader.read_with_variant_length();

    return MaxStreamFrame {
        .is_bidirectional = is_bidirectional,
        .is_block = is_block,
        .max_stream = max_stream,
    };
}




//
// Created by Chengke Wong on 2020/4/29.
//

#ifndef SEND_BUFFER__H
#define SEND_BUFFER__H

#include <map>

#include "util/string_raw.h"
#include "util/string_writer.h"

class SendBuffer {

public: 

    // new data from application.
    // return the number of bytes that are inserted
    size_t insert(StringRef data);

    void mark_as_lost(uint64_t offset, size_t length);

    void mark_as_acked(uint64_t offset, size_t length);

    // sent bytes to network
    void next_bytes(StringWriter &writer);

private:

    void mark_as_sent(uint64_t offset, size_t length);

    enum class Status {
        UNSENT, // or lost
        SENT,
        ACKED,
    }

    using DataMap = std::map<uint64_t, Status>;

    SendBuffer (SendBuffer&&) = default;
    SendBuffer& operator= (SendBuffer&&) = default;

    // disallow copy and assignment
    SendBuffer (const SendBuffer&) = delete;
    SendBuffer& operator= (const SendBuffer&) = delete;

};

#endif

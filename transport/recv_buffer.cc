//
// Created by Chengke Wong on 2020/4/28.
//

#include "transport/recv_buffer.h"

#include <algorithm>

#include "util/string_writer.h"

RecvBuffer::RecvBuffer()
    : data_(),
      retired_(0)
{}

void RecvBuffer::insert(uint64_t offset, StringRef data) {
    uint64_t new_begin = std::max(offset, retired_);
    uint64_t new_end = offset + data.size();

    if (new_begin >= new_end) {
        return;
    }

    DataMap::iterator iter = data_.upper_bound(offset);
    if (iter != data_.begin()) {
        DataMap::iterator iter_lower = iter;
        iter_lower--;
        new_begin = std::max(new_begin, 
            iter_lower->first + iter_lower->second.size());
    }

    while (new_begin < new_end) {
        if (iter == data_.end() || iter->first >= new_end) {
            insert_slice(offset, data, new_begin, new_end);
            return;
        }

        insert_slice(offset, data, new_begin, iter->first);
        new_begin = iter->first + iter->second.size();
        iter++;
    }
}

bool RecvBuffer::readable() const {
    if (data_.empty()) {
        return false;
    }
    return data_.begin()->first == retired_;
}

size_t RecvBuffer::read_to_end(StringRef output) {
    DataMap::iterator iter = data_.begin();
    StringWriter writer(output);
    while (iter != data_.end()) {
        if (iter->first > retired_) {
            return writer.position();
        }

        uint64_t iter_truncated = retired_ - iter->first;
        size_t readable = iter->second.size() - iter_truncated;
        readable = std::min(readable, writer.remaining());

        writer.write(iter->second.data() + iter_truncated, readable);
        retired_ += readable;

        if (retired_ >= iter->first + iter->second.size()) {
            DataMap::iterator iter_next = iter;
            iter_next++;
            data_.erase(iter);
            iter = iter_next;
        } else {
            return writer.position();
        }
    }
    return writer.position();
}

void RecvBuffer::insert_slice(uint64_t offset, StringRef data, 
                              uint64_t begin, uint64_t end) {
    // LOG(DEBUG) << begin << ", " << end;
    String new_data(&data[begin - offset], end - begin);
    data_.insert(std::make_pair(begin, std::move(new_data)));
}

//
// Created by Chengke Wong on 2020/4/28.
//

#ifndef RECV_BUFFER_H
#define RECV_BUFFER_H

#include <map>

#include "util/string_raw.h"

class RecvBuffer {

public: 

    RecvBuffer();

    // This method will make a copy of the |data|.
    void insert(uint64_t offset, StringRef data);

    bool readable() const;

    // return how many bytes are read
    size_t read_to_end(StringRef output);

    inline uint64_t retired() const {
        return retired_;
    }

    // disallow copy and assignment
    RecvBuffer (const RecvBuffer&) = delete;
    RecvBuffer& operator=(const RecvBuffer&) = delete;

    RecvBuffer (RecvBuffer&&) = delete;
    RecvBuffer& operator=(RecvBuffer&&) = delete;

private:

    // only insert the data whose sequence numbers are 
    // between |begin| and |end|
    void insert_slice(uint64_t offset, StringRef data, 
        uint64_t begin, uint64_t end);

    using DataMap = std::map<uint64_t, String>;

    // sort by offset
    DataMap data_;

    // number of bytes buffered in |data_|
    // uint64_t buffered_;

    // number of bytes the application has read
    uint64_t retired_; 

};

#endif

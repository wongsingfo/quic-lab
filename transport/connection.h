//
// Created by Chengke Wong on 2020/6/28.
//

#ifndef QUIC_LAB_CONNECTION_H
#define QUIC_LAB_CONNECTION_H

#include "util/string_raw.h"

class Connection {
public: 

  Connection(AlarmFactory *alarm_factory);

  void process_input(StringRef packet) noexcept;

  // build a output packet. 
  // if nothing is sent, return a null string
  StringRef process_output() noexcept

  void close_output();

  // stream manipulations:

  StreamId stream_create();

  size_t stream_send(StreamId id, StringRef data);

  void stream_close(StreamId id);

  size_t stream_receive(StreamId id, StringRef data);

  // disallow copy and assignment
  Connection (const Connection&) = delete;
  Connection& operator= (const Connection&) = delete;

};

#endif //QUIC_LAB_CONNECTION_H

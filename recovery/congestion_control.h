//
// Created by Chengke Wong on 2020/04/30.
//

#ifndef CONGESTION_CONTROL_H
#define CONGESTION_CONTROL_H

#include "recovery/sent_packet.h"

// CC interface
class CongestionControl {

public: 

    virtual ~CongestionControl() = default;

    virtual void on_packet_sent(unique_ptr<SentPacket> &packet) = 0;

    virtual void on_packet_lost(std::vector<unique_ptr<SentPacket>> &packets) = 0;

    virtual void on_packet_acked(std::vector<unique_ptr<SentPacket>> &packets) = 0;

private:

    CongestionControl (CongestionControl&&) = default;
    CongestionControl& operator= (CongestionControl&&) = default;

    // disallow copy and assignment
    CongestionControl (const CongestionControl&) = delete;
    CongestionControl& operator= (const CongestionControl&) = delete;

};

#endif



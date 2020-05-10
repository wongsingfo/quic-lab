//
// Created by Chengke Wong on 2020/04/30.
//

#ifndef CONGESTION_CONTROL_H
#define CONGESTION_CONTROL_H

#include <ostream>

#include "recovery/sent_packet.h"

// CC interface
class CongestionControl {

public: 

    virtual ~CongestionControl() = default;

    virtual void on_packet_sent(unique_ptr<SentPacket> &packet) = 0;

    virtual void on_packet_lost(Instant now, std::vector<unique_ptr<SentPacket>> &packets) = 0;

    virtual void on_packet_acked(std::vector<unique_ptr<SentPacket>> &packets) = 0;

    virtual void print(std::ostream& os) {
    	os << "unknown cc";
    }

    friend std::ostream& operator<< (std::ostream& out, CongestionControl& self) {
	    self.print(out);
	    return out;
	}

};

#endif



//
// Created by Chengke Wong on 2020/5/10.
//

#include "recovery/cc_cubic.h"

CcCubic::CcCubic()
        : cc_window_(kInitialWindow),
          bytes_in_flight_(0),
          congestion_recovery_start_time_(Instant::zero()),
          ssthresh_(std::numeric_limits<size_t>::max()) {

}

void CcCubic::print(std::ostream &os) {
    os << "cubic "
       << bytes_in_flight_ << "/" << cc_window_
       << " ssthresh " << ssthresh_;
}

void CcCubic::on_packet_sent(unique_ptr<SentPacket> &packet) {
    bytes_in_flight_ += packet->sent_bytes;
}

void CcCubic::on_packet_acked(std::vector<unique_ptr<SentPacket>> &packets) {
    for (auto &packet : packets) {
        bytes_in_flight_ -= packet->sent_bytes;

        if (in_congestion_recovery(packet->time_sent)) {
            // Do not increase congestion window in recovery period.
            continue;
        }

// TODO:
        // if (IsAppOrFlowControlLimited()):
        //          // Do not increase congestion_window if application
        //          // limited or flow control limited.
        //          continue

        if (cc_window_ < ssthresh_) {
            // Slow start.
            cc_window_ += packet->sent_bytes;
        } else {
            // Congestion avoidance.
            // TODO: division operation may be slow
            cc_window_ += (kMaxDatagramSize * packet->sent_bytes) / cc_window_;
        }
    }
}

void CcCubic::on_packet_lost(Instant now, std::vector<unique_ptr<SentPacket>> &packets) {
    DCHECK(!packets.empty());

    Instant lastest = packets.front()->time_sent;

    for (auto &packet : packets) {
        bytes_in_flight_ -= packet->sent_bytes;
        lastest = std::max(lastest, packet->time_sent);
    }

    on_congestion_event(now, lastest);

    // TODO:
    // // Collapse congestion window if persistent congestion
    //     if (InPersistentCongestion(lost_packets)):
    //       congestion_window = kMinimumWindow
}

bool CcCubic::in_congestion_recovery(Instant sent_time) {
    return sent_time <= congestion_recovery_start_time_;
}

void CcCubic::on_congestion_event(Instant now, Instant sent_time) {
    if (not in_congestion_recovery(sent_time)) {
        return;
    }

    congestion_recovery_start_time_ = now;

    // congestion_window *= kLossReductionFactor
    // congestion_window = max(congestion_window, kMinimumWindow)
    // Here, we let kLossReductionFactor = 0.5
    cc_window_ = std::max(cc_window_ >> 1, kMinimumWindow);

    ssthresh_ = cc_window_;

    // TODO:
    // A packet can be sent to speed up loss recovery.
    // MaybeSendOnePacket()
}



//
// Created by Chengke Wong on 2020/4/30.
//

#ifndef LOSS_RECOVERY_H
#define LOSS_RECOVERY_H

#include <tuple>

#include "common/frame.h"
#include "common/quic_types.h"
#include "recovery/sent_packet.h"
#include "recovery/cc.h"
#include "recovery/rtt_time.h"
#include "util/alarm.h"

class LossRecoverySpace {

public:

    // Maximum reordering in packets before packet threshold loss detection
    // considers a packet lost.
    // Implementations SHOULD NOT use a packet threshold less than 3, to keep
    // in line with TCP [RFC5681].
    static constexpr size_t kPacketThreshold = 3;

    void on_packet_sent(PacketNumber pn, unique_ptr<SentPacket> packet);

    // return whether the largest_acked_packet is updated
    bool update_largest_acked_packet(AckFrame &ack);

    // return newly_acked_packets and a boolean indicating whether the newly
    // acked packets contains a ack-eliciting one
    std::vector<unique_ptr<SentPacket>>
    detect_and_remove_acked_packets(AckFrame &ack);

    std::vector<unique_ptr<SentPacket>>
    detect_and_remove_lost_packets(Duration loss_delay,
                                   Instant now);

    inline Instant loss_time() const {
        return loss_time_;
    }

    inline Instant time_of_last_sent_ack_eliciting_packet() const {
        return time_of_last_sent_ack_eliciting_packet_;
    }

private:

    optional<PacketNumber> largest_acked_packet_;

    using SentPackets = std::map<PacketNumber, unique_ptr<SentPacket> >;

    SentPackets sent_packets_;

    size_t ack_eliciting_outstanding_ = 0;

    Instant time_of_last_sent_ack_eliciting_packet_ = Instant::infinite();;

    // The time at which the next packet in that packet number space will be
    // considered lost based on exceeding the reordering window in time.
    Instant loss_time_ = Instant::infinite();
};

class LossRecoverySpaces {
public:

    inline LossRecoverySpace &operator [] (PNSpace space) {
        return spaces_[static_cast<size_t>(space)];
    }

    inline LossRecoverySpace &operator [] (size_t space) {
        return spaces_[space];
    }

private:

    LossRecoverySpace spaces_[kNumOfPNSpaces];

};

class LossRecovery {

public: 

    void on_packet_sent(PNSpace space, PacketNumber pn, unique_ptr<SentPacket> packet);

    void on_ack_received(PNSpace space, AckFrame &ack, Instant now);

    // disallow copy and assignment
    LossRecovery (const LossRecovery&) = delete;
    LossRecovery& operator= (const LossRecovery&) = delete;

    LossRecovery (LossRecovery&&) = default;
    LossRecovery& operator= (LossRecovery&&) = default;

private:

    bool is_handshake_complete_;

    RttTime rtt_time_;

    LossRecoverySpaces spaces_;

    // The number of times a PTO has been sent without receiving an ack
    size_t pto_count_;
    static constexpr size_t kPtoCountLimit = 10;

    unique_ptr<CongestionControl> cc_;

    static bool includes_ack_eliciting(std::vector<unique_ptr<SentPacket>> &acked_packet);

    bool peer_completed_address_validation();

    void set_loss_detection_alarm(Instant now);

    void on_loss_detection_timeout(Instant now);

    using WhatEarliestTime = Instant (LossRecoverySpace::*)() const;
    std::tuple<Instant, PNSpace> 
    get_earliest_time_and_space(WhatEarliestTime time);

    unique_ptr<Alarm> loss_detection_alarm_;

};

#endif

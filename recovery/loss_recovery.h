//
// Created by Chengke Wong on 2020/4/30.
//

#ifndef LOSS_RECOVERY_H
#define LOSS_RECOVERY_H

#include "common/frame.h"
#include "common/types.h"
#include "recovery/sent_packet.h"
#include "recovery/congestion_control.h"
#include "recovery/rtt_time.h"

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
    detect_and_remove_lost_packets(AckFrame &ack, Duration loss_delay,
                                   Instant now);

private:

    optional<PacketNumber> largest_acked_packet_;

    Instant time_of_last_sent_ack_eliciting_packet_;

    using SentPackets = std::map<PacketNumber, unique_ptr<SentPacket> >;

    SentPackets sent_packets_;

    size_t ack_eliciting_outstanding_ = 0;

    // The time at which the next packet in that packet number space will be
    // considered lost based on exceeding the reordering window in time.
    Instant loss_time_ = Instant::infinite();
};

class LossRecoverySpaces {
public:

    inline LossRecoverySpace &operator [] (PNSpace space) {
        return spaces_[static_cast<size_t>(space)];
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

    RttTime rtt_time_;

    LossRecoverySpaces spaces_;

    size_t pto_count_;

    unique_ptr<CongestionControl> cc_;

    static bool includes_ack_eliciting(std::vector<unique_ptr<SentPacket>> &acked_packet);

    bool peer_completed_address_validation();

};

#endif

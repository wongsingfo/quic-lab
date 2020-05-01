//
// Created by Chengke Wong on 2020/5/1.
//

#include "recovery/loss_recovery.h"

#include <algorithm>

void LossRecoverySpace::on_packet_sent(PacketNumber pn,
                                       unique_ptr<SentPacket> packet) {
    if (packet->ack_eliciting) {
        time_of_last_sent_ack_eliciting_packet_ = packet->time_sent;
        ack_eliciting_outstanding_ += 1;
    }

    sent_packets_.insert(std::make_pair(pn, std::move(packet)));
}

bool LossRecoverySpace::update_largest_acked_packet(AckFrame &ack) {
    if (largest_acked_packet_.has_value()) {
        if (ack.largest_ack > largest_acked_packet_.value()) {
            largest_acked_packet_ = ack.largest_ack;
            return true;
        } else {
            return false;
        }
    } else {
        largest_acked_packet_ = ack.largest_ack;
        return true;
    }
}

std::vector<unique_ptr<SentPacket>>
LossRecoverySpace::detect_and_remove_acked_packets(AckFrame &ack) {
    std::vector<unique_ptr<SentPacket> > acked_packets;

    for (AckRange range: ack.ranges) {
        uint64_t pn_start = range.start;
        uint64_t pn_end = range.start + range.length;

        for (uint64_t pn = pn_start; pn < pn_end; pn++) {
            auto iter = sent_packets_.find(pn);
            if (iter != sent_packets_.end()) {
                acked_packets.push_back(std::move(iter->second));
                sent_packets_.erase(iter);
            }
        }
    }
    return acked_packets;
}

std::vector<unique_ptr<SentPacket>>
LossRecoverySpace::detect_and_remove_lost_packets(AckFrame &ack, Duration loss_delay,
                                                  Instant now) {
    // https://quicwg.org/base-drafts/draft-ietf-quic-recovery.html#name-detecting-lost-packets
    std::vector<unique_ptr<SentPacket>> lost_packets;

    dynamic_check(largest_acked_packet_.has_value());
    PacketNumber largest_acked_packet = largest_acked_packet_.value();

    Instant lost_send_time = now - loss_delay;
    loss_time_ = Instant::infinite();

    auto iter = sent_packets_.begin();
    while (iter != sent_packets_.end()) {
        PacketNumber unacked_packet_number = iter->first;
        unique_ptr<SentPacket> &unacked = iter->second;
        if (unacked_packet_number > largest_acked_packet_) {
            continue;
        }

        // A packet is declared lost
        if (unacked->time_sent <= lost_send_time ||
            largest_acked_packet_->value >=
                unacked_packet_number.value + kPacketThreshold) {

            if (unacked->in_flight) {
                lost_packets.push_back(std::move(unacked));
            }

            auto iter_to_be_erased = iter;
            iter++;
            sent_packets_.erase(iter_to_be_erased);
        } else {
            loss_time_ = std::min(loss_time_, unacked->time_sent + loss_delay);
        }
    }

    return lost_packets;
}

void LossRecovery::on_packet_sent(PNSpace space, PacketNumber pn,
                                  unique_ptr<SentPacket> packet) {
    // https://quicwg.org/base-drafts/draft-ietf-quic-recovery.html#name-on-sending-a-packet
    cc_->on_packet_sent(packet);
    spaces_[space].on_packet_sent(pn, std::move(packet));
}

void LossRecovery::on_ack_received(PNSpace space, AckFrame &ack, Instant now) {
    // https://quicwg.org/base-drafts/draft-ietf-quic-recovery.html#name-on-receiving-an-acknowledgm
    LossRecoverySpace &recovery_space = spaces_[space];

    auto newly_acked_packets =
        recovery_space.detect_and_remove_acked_packets(ack);

    if (newly_acked_packets.empty()) {
        return;
    }

    // If the largest acknowledged is newly acked and
    // at least one ack-eliciting was newly acked, update the RTT.
    bool newly_ack_the_largest = recovery_space.update_largest_acked_packet(ack);
    if (newly_ack_the_largest && includes_ack_eliciting(newly_acked_packets)) {
        unique_ptr<SentPacket> &largest_acked_pkt = newly_acked_packets.back();
        Duration latest_rtt = now - largest_acked_pkt->time_sent;
        Duration ack_delay = Duration::zero();

        if (space == PNSpace::Application) {
            ack_delay = Duration::from_microseconds(ack.ack_delay);
        }
        rtt_time_.update_rtt(latest_rtt, ack_delay);
    }

    // TODO: Process ECN information if present.
    //  if (ACK frame contains ECN information):
    //      ProcessECN(ack, pn_space)

    auto lost_packets =
        recovery_space.detect_and_remove_lost_packets(
            ack, rtt_time_.loss_delay(), now);

    if (!lost_packets.empty()) {
        cc_->on_packet_lost(lost_packets);
    }
    cc_->on_packet_acked(newly_acked_packets);

    // Reset pto_count unless the client is unsure if
    // the server has validated the client's address.
    if (peer_completed_address_validation()) {
        pto_count_ = 0;
    }

    // SetLossDetectionTimer()
}

bool LossRecovery::includes_ack_eliciting(
    std::vector<unique_ptr<SentPacket>> &acked_packet) {
    for (auto &packet : acked_packet) {
        if (packet->ack_eliciting) {
            return true;
        }
    }
    return false;
}

bool LossRecovery::peer_completed_address_validation() {
    // TODO
    //  # Assume clients validate the server's address implicitly.
    //  if (endpoint is server):
    //    return true
    //  # Servers complete address validation when a
    //  # protected packet is received.
    //  return has received Handshake ACK ||
    //       has received 1-RTT ACK ||
    //       has received HANDSHAKE_DONE
    return true;
}

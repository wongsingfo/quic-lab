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
LossRecoverySpace::detect_and_remove_lost_packets(Duration loss_delay,
                                                  Instant now) {
    // https://quicwg.org/base-drafts/draft-ietf-quic-recovery.html#name-detecting-lost-packets
    std::vector<unique_ptr<SentPacket>> lost_packets;

    DCHECK(largest_acked_packet_.has_value());
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
    if (! packet->in_flight) {
        return;
    }

    Instant now = packet->time_sent;

    // https://quicwg.org/base-drafts/draft-ietf-quic-recovery.html#name-on-sending-a-packet
    cc_->on_packet_sent(packet);
    spaces_[space].on_packet_sent(pn, std::move(packet));
    set_loss_detection_alarm(now);
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
            rtt_time_.loss_delay(), now);

    if (!lost_packets.empty()) {
        cc_->on_packet_lost(lost_packets);
    }
    cc_->on_packet_acked(newly_acked_packets);

    // Reset pto_count unless the client is unsure if
    // the server has validated the client's address.
    if (peer_completed_address_validation()) {
        pto_count_ = 0;
    }

    set_loss_detection_alarm(now);
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

void LossRecovery::set_loss_detection_alarm(Instant now) {
    // https://quicwg.org/base-drafts/draft-ietf-quic-recovery.html#name-setting-the-loss-detection-
    // GetEarliestTimeAndSpace
    Instant earliest_loss_time = std::get<0>(
        get_earliest_time_and_space(&LossRecoverySpace::loss_time));

    // Time threshold loss Detection
    if (!earliest_loss_time.is_infinite()) {
        loss_detection_alarm_->update(earliest_loss_time);
        return;
    }

    // TODO:
    // if (server is at anti-amplification limit):
    //  The server's alarm is not set if nothing can be sent.
    //  loss_detection_timer.cancel()
    //  return

    // TODO:
    // if (no ack-eliciting packets in flight &&
    //     PeerCompletedAddressValidation()):
    //   // There is nothing to detect lost, so no timer is set.
    //   // However, the client needs to arm the timer if the
    //   // server might be blocked by the anti-amplification limit.
    //   loss_detection_timer.cancel()
    //   return

    // Determine which PN space to arm PTO for.
    PNSpace pn_space;
    Instant sent_time = Instant::infinite();
    std::tie(sent_time, pn_space) = get_earliest_time_and_space(
        &LossRecoverySpace::time_of_last_sent_ack_eliciting_packet);

    // Don't arm PTO for ApplicationData until handshake complete.
    if (pn_space == PNSpace::Application && !is_handshake_complete_) {
        loss_detection_alarm_->cancel();
    }

    if (sent_time.is_infinite()) {
        DCHECK(!peer_completed_address_validation());
        sent_time = now;
    }

    // Calculate PTO duration
    Duration timeout = rtt_time_.pto();
    timeout = timeout << std::min(pto_count_, size_t(kPtoCountLimit));
    loss_detection_alarm_->update(sent_time + timeout);
}

void LossRecovery::on_loss_detection_timeout(Instant now) {
    // https://quicwg.org/base-drafts/draft-ietf-quic-recovery.html#name-on-timeout
    std::tuple<Instant, PNSpace> earliest_loss = 
        get_earliest_time_and_space(&LossRecoverySpace::loss_time);

    // Time threshold loss Detection
    if (!std::get<0>(earliest_loss).is_infinite()) {
        auto lost_packets = 
            spaces_[std::get<1>(earliest_loss)].detect_and_remove_lost_packets(
                rtt_time_.loss_delay(), now);

        DCHECK(!lost_packets.empty());
        cc_->on_packet_lost(lost_packets);

        set_loss_detection_alarm(now);
        return;
    }

    // TODO:
    //   IMPORTANT:
    // if (bytes_in_flight > 0):
    //     // PTO. Send new data if available, else retransmit old data.
    //     // If neither is available, send a single PING frame.
    //     _, pn_space = GetEarliestTimeAndSpace(
    //       time_of_last_sent_ack_eliciting_packet)
    //     SendOneOrTwoAckElicitingPackets(pn_space)
    //   else:
    //     assert(endpoint is client without 1-RTT keys)
    //     // Client sends an anti-deadlock packet: Initial is padded
    //     // to earn more anti-amplification credit,
    //     // a Handshake packet proves address ownership.
    //     if (has Handshake keys):
    //       SendOneAckElicitingHandshakePacket()
    //     else:
    //       SendOneAckElicitingPaddedInitialPacket()

    pto_count_ += 1;
    set_loss_detection_alarm(now);
}

std::tuple<Instant, PNSpace> 
LossRecovery::get_earliest_time_and_space(WhatEarliestTime what) {
    PNSpace space = PNSpace::Initial;
    Instant time = (spaces_[0].*what)();

    for (int i = 1; i < kNumOfPNSpaces; i++) {
        PNSpace pn_space = static_cast<PNSpace>(i);
        Instant pn_space_time = (spaces_[i].*what)();
        if (pn_space_time < time && 
            // Skip ApplicationData until handshake completion.
            (pn_space != PNSpace::Application || is_handshake_complete_)) {

            time = pn_space_time;
            space = pn_space;
        }
    }

    return std::make_tuple(time, space);
}


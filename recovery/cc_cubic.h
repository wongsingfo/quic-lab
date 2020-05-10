//
// Created by Chengke Wong on 2020/5/10.
//

#ifndef RECOVERY_CC_CUBIC_H
#define RECOVERY_CC_CUBIC_H

#include "common/config.h"
#include "recovery/cc.h"
#include "util/time.h"

class CcCubic : public CongestionControl {

public:

    CcCubic();

	~CcCubic() override = default;

    void on_packet_sent(unique_ptr<SentPacket> &packet) override;

    void on_packet_lost(Instant now, std::vector<unique_ptr<SentPacket>> &packets) override;

    void on_packet_acked(std::vector<unique_ptr<SentPacket>> &packets) override;

    void print(std::ostream& os) override;

private:

	static constexpr size_t kMinimumWindow = kMaxDatagramSize * 2;

	static constexpr size_t kInitialCwndPackets = 10;

	static constexpr size_t kInitialWindow = const_min(kInitialCwndPackets * kMaxDatagramSize,
                                                       const_max(14720ul, kMaxDatagramSize * 2));

	static constexpr size_t kPersistentCongestionThreshold = 3;

	void on_congestion_event(Instant now, Instant sent_time);

	bool in_congestion_recovery(Instant sent_time);

	// Congestion window size in bytes.
	size_t cc_window_;

	// Slow start threshold in bytes. When the congestion window is below ssthresh, 
	// the mode is slow start and the window grows by the number of bytes acknowledged.
	size_t ssthresh_;

	size_t bytes_in_flight_;

	// The time when QUIC first detects congestion due to loss or ECN, causing it to 
	// enter congestion recovery. When a packet sent after this time is acknowledged, 
	// QUIC exits congestion recovery.
	Instant congestion_recovery_start_time_;

};

#endif //RECOVERY_CC_CUBIC_H

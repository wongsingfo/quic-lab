//
// Created by Chengke Wong on 2020/5/1.
//

#include "recovery/rtt_time.h"

#include <algorithm>

#include "common/config.h"
#include "util/utility.h"

// https://quicwg.org/base-drafts/draft-ietf-quic-recovery.html#name-estimating-smoothed_rtt-and
void RttTime::update_rtt(Duration rtt_sample, Duration ack_delay) {
    latest_rtt_ = rtt_sample;

    min_rtt_ = std::min(min_rtt_, rtt_sample);

    // smoothed_rtt and rttvar are computed as follows, similar to [RFC6298].
    if (no_samples_) {
        no_samples_ = false;

        smoothed_rtt_ = rtt_sample;
        rttvar_ = rtt_sample >> 1;
    } else {
        ack_delay = std::min(ack_delay, max_ack_delay_);
        if (min_rtt_ + ack_delay < rtt_sample) {
            rtt_sample = rtt_sample - ack_delay;
        }

        smoothed_rtt_ = ((smoothed_rtt_ * 7) + rtt_sample) >> 3;
        Duration rttvar_sample = (smoothed_rtt_ - rtt_sample).abs();
        rttvar_ = (rttvar_ * 3 + rttvar_sample) >> 2;
    }
}

Duration RttTime::loss_delay() {
    dynamic_check(! no_samples_);

    // kTimeThreshold = 9 / 8
    // Maximum reordering in time before time threshold loss detection considers
    // a packet lost. Specified as an RTT multiplier.

    Duration result = (std::max(latest_rtt_, smoothed_rtt_) * 9) >> 3;
    return std::max(kTimerGranularity, result);
}

Duration RttTime::pto() {
    return smoothed_rtt_ 
        + std::max(4 * rttvar_, kTimerGranularity) 
        + max_ack_delay_;
}

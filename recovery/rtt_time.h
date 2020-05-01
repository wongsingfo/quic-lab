//
// Created by Chengke Wong on 2020/5/1.
//

#ifndef RTT_TIME_H
#define RTT_TIME_H

#include "util/time.h"

// Estimating the Round-Trip Time
// This class is not for generating RTT samples
class RttTime {

public: 

    RttTime(Duration max_ack_delay) 
        : max_ack_delay_(max_ack_delay),
          min_rtt_(Duration::infinite()),
          smoothed_rtt_(Duration::zero()),
          rttvar_(Duration::zero()),
          no_samples_(true)
        {}

    // |ack_delay| is carried in the ACK frame
    void update_rtt(Duration rtt_sample, Duration ack_delay);

private:

    RttTime (RttTime&&) = default;
    RttTime& operator= (RttTime&&) = default;

    // disallow copy and assignment
    RttTime (const RttTime&) = delete;
    RttTime& operator= (const RttTime&) = delete;

    // transport parameter
    Duration max_ack_delay_;

    // minimum RTT observed for a given network path.
    //
    // min_rtt is set to the latest_rtt on the first RTT sample, 
    // and to the lesser of min_rtt and latest_rtt on subsequent 
    // samples.
    // 
    // min_rtt is used by loss detection to reject implausibly 
    // small rtt samples.
    Duration min_rtt_;

    // an exponentially-weighted moving average of an endpoint's 
    // RTT samples
    Duration smoothed_rtt_;

    // the variation in the RTT samples, estimated using a mean 
    // variation.
    Duration rttvar_;

    bool no_samples_;

};

#endif

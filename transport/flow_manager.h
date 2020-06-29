//
// Created by Chengke Wong on 2020/4/29.
//

#ifndef FLOW_MANAGER__H
#define FLOW_MANAGER__H

// Flow Control
// - prevent a fast sender from overwhelming a slow receiver
// - prevent a malicious sender from consuming a large amount 
//   of memory at a receiver.

// Data sent in CRYPTO frames is not flow controlled in the 
// same way as stream data. The [QUIC-TLS] implementation SHOULD 
// provide an interface to QUIC to tell it about its buffering limits 
// so that there is not excessive buffering at multiple layers.

class FlowManager {

public: 


private:

    FlowManager (FlowManager&&) = default;
    FlowManager& operator= (FlowManager&&) = default;

    // disallow copy and assignment
    FlowManager (const FlowManager&) = delete;
    FlowManager& operator= (const FlowManager&) = delete;

};

#endif

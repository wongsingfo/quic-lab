//
// Created by Chengke Wong on 2020/4/29.
//

#ifndef RECOVERY_TOKEN_H
#define RECOVERY_TOKEN_H

#include <vector>

#include "util/utility.h"

// interface
class RecoverToken {

public: 

    virtual ~RecoverToken() = default;
    
    virtual void on_acked() = 0;

    virtual void on_lost() = 0;

private:

};

using RecoverTokens = std::vector< unique_ptr<RecoverToken> >;

#endif

//
// Created by Chengke Wong on 2020/4/9.
//

#ifndef TRANSPORT_EXCEPTION_H
#define TRANSPORT_EXCEPTION_H

#include <exception>

class error_discard_packet : public std::runtime_error {

public:
    template<typename... Args>
    explicit error_discard_packet(Args&&... args)
        : std::runtime_error(std::forward<Args>(args)...)
        {}

};

#endif //TRANSPORT_EXCEPTION_H

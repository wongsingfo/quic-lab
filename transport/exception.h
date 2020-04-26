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

// https://quicwg.org/base-drafts/draft-ietf-quic-transport.html#name-quic-transport-error-codes-
enum class QuicError {
    INTERNAL_ERROR = 0x1,
    FRAME_ENCODING_ERROR = 0x7,
};

class quic_error : public std::exception {

public: 
    explicit quic_error(QuicError error) 
        : std::exception(), error_(error) {}

private:

    QuicError error_;

};

class error_protocol_violation : public std::runtime_error {

public:
    template<typename... Args>
    explicit error_protocol_violation(Args&&... args)
        : std::runtime_error(std::forward<Args>(args)...)
    {}

};

#endif //TRANSPORT_EXCEPTION_H

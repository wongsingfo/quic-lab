//
// Created by Chengke Wong on 2020/5/10.
//

#ifndef QUIC_LAB_EXCEPTION_SSL_H
#define QUIC_LAB_EXCEPTION_SSL_H

#include <openssl/err.h>
#include "exception.h"

/* error category for OpenSSL */
class openssl_error_category : public std::error_category {
public:
    const char *name() const noexcept override { return "SSL"; }

    std::string message(const int ssl_error) const noexcept override {
        return ERR_error_string(ssl_error, nullptr);
    }
};

class openssl_error : public tagged_error {
public:
    explicit openssl_error(const std::string &s_attempt, const int error_code = ERR_get_error())
            : tagged_error(openssl_error_category(), s_attempt, error_code) {}
};

inline int openssl_call(const std::string &s_attempt, int return_value) {
    if (return_value > 0) {
        return return_value;
    }
    throw openssl_error(s_attempt);
}

#endif //QUIC_LAB_EXCEPTION_SSL_H

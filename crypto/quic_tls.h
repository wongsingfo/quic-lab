//
// Created by Chengke Wong on 2020/4/24.
//

// see quic_tls.api.h for details

#ifndef CRYPTO_QUIC_TLS_H
#define CRYPTO_QUIC_TLS_H

#include "openssl/base.h"

#include "crypto/exception.h"
#include "util/string.h"

class QuicTls;

enum class EncryptionLevel {
    InitialKey = 0,
    EarlyDataKey = 1, // 0-RTT key
    HandshakeKey = 2,
    ApplicationKey = 3,
};

class QuicTlsContext {
    friend QuicTls;

public:
    QuicTlsContext();

    ~QuicTlsContext();

    void load_verify_locations_from_file(const char* file);

    void load_verify_locations_from_path(const char* path);

private:
    SSL_CTX *ctx_;

};

class QuicTls {

public:
    QuicTls(QuicTlsContext ctx, bool is_server);

    ~QuicTls();

    void set_transport_params(StringRef data);

private:
    SSL *ssl_;
};

#endif //CRYPTO_QUIC_TLS_H

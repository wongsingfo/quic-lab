//
// Created by Chengke Wong on 2020/4/24.
//

// see quic_tls.api.h for details

#ifndef CRYPTO_QUIC_TLS_H
#define CRYPTO_QUIC_TLS_H

#include <functional>

#include "openssl/base.h"

#include "util/exception_ssl.h"
#include "util/string.h"
#include "crypto/aead.h"
#include "crypto/cipher.h"

class QuicTls;

namespace crypto {


} // namespace crypto

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
    using SecretCallback = std::function<void(EncryptionLevel level, bool is_read, Cipher cipher)>;
    using DataCallback = std::function<void(EncryptionLevel level, StringRef data)>;

    QuicTls(QuicTlsContext ctx, bool is_server, 
        const SecretCallback &secret_callback, 
        const DataCallback &data_callback);

    ~QuicTls();

    void provide_data(EncryptionLevel level, StringRef data);

    void set_transport_params(StringRef data);

    inline SecretCallback get_secret_callback() const {
        return secret_callback_;
    }

    inline DataCallback get_data_callback() const {
        return data_callback_;
    }

private:

    SSL *ssl_;

    SecretCallback secret_callback_;
    DataCallback data_callback_;

};

#endif //CRYPTO_QUIC_TLS_H

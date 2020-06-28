//
// Created by Chengke Wong on 2020/4/24.
//

#include "openssl/ssl.h"

#include "quic_tls.h"
#include "util/string_raw.h"
#include "crypto/aead.h"

using namespace crypto;

namespace crypto {

// https://commondatastorage.googleapis.com/chromium-boringssl-docs/ssl.h.html#QUIC-integration

static int QUIC_EX_DATA_INDEX =
    SSL_get_ex_new_index(0, nullptr, nullptr, nullptr, nullptr);

static CipherSuite get_cipher_suite(const SSL_CIPHER *cipher) {
    // https://testssl.sh/openssl-iana.mapping.html

    int id = SSL_CIPHER_get_id(cipher);
    switch (id) {
        case 0x03001301:
            return CipherSuite::TLS_AES_128_GCM_SHA256;
        case 0x03001302:
            return CipherSuite::TLS_AES_256_GCM_SHA384;
        case 0x03001303:
            return CipherSuite::TLS_CHACHA20_POLY1305_SHA256;
        default:
            throw openssl_error("convert to iana", 0);
    }
}

static void set_secret_key(SSL *ssl, ssl_encryption_level_t level,
        const SSL_CIPHER *cipher, const uint8_t *secret,
        size_t secret_len, bool is_read) {
    QuicTls *self = static_cast<QuicTls*>(SSL_get_ex_data(ssl, QUIC_EX_DATA_INDEX));
    EncryptionLevel e_level = (EncryptionLevel) level;

    StringRef secret_key((StringRef::dtype*) secret, secret_len);
    Cipher ciphersuite(get_cipher_suite(cipher), secret_key);

    (self->get_secret_callback())(e_level, is_read, std::move(ciphersuite));
}

static int set_read_secret(SSL *ssl, ssl_encryption_level_t level,
        const SSL_CIPHER *cipher, const uint8_t *secret,
        size_t secret_len) {
    set_secret_key(ssl, level, cipher, secret, secret_len, true);
    return 1 /* success */;
}

static int set_write_secret(SSL *ssl, ssl_encryption_level_t level,
        const SSL_CIPHER *cipher, const uint8_t *secret,
        size_t secret_len) {
    set_secret_key(ssl, level, cipher, secret, secret_len, false);
    return 1 /* success */;
}

static int add_handshake_data(SSL *ssl, enum ssl_encryption_level_t level,
                            const uint8_t *data, size_t len) {
    QuicTls *self = static_cast<QuicTls*>(SSL_get_ex_data(ssl, QUIC_EX_DATA_INDEX));
    EncryptionLevel e_level = (EncryptionLevel) level;

    StringRef data_s((StringRef::dtype*) data, len);
    (self->get_data_callback())(e_level, data_s);
    return 1 /* success */;
}

static int flush_flight(SSL *ssl) {
    /* ignore this message now */

    return 1 /* success */;
}

static int send_alert(SSL *ssl, enum ssl_encryption_level_t level, uint8_t alert) {
    /* ignore this message now */

    return 1 /* success */;
}

static SSL_QUIC_METHOD quic_method = {
    .set_read_secret = set_read_secret,
    .set_write_secret = set_write_secret,
    .add_handshake_data = add_handshake_data,
    .flush_flight = flush_flight,
    .send_alert = send_alert,
};

} // namespace crypto

QuicTlsContext::QuicTlsContext()
    : ctx_(SSL_CTX_new(TLS_method())) {
    openssl_call("set_default_verify_paths",
                 SSL_CTX_set_default_verify_paths(ctx_));
}

QuicTlsContext::~QuicTlsContext() {
    SSL_CTX_free(ctx_);
}

void QuicTlsContext::load_verify_locations_from_file(const char *file) {
    openssl_call("load verify location",
                 SSL_CTX_load_verify_locations(ctx_, file, nullptr));
}

void QuicTlsContext::load_verify_locations_from_path(const char *path) {
    openssl_call("load verify location",
                 SSL_CTX_load_verify_locations(ctx_, nullptr, path));
}

QuicTls::QuicTls(QuicTlsContext &ctx, bool is_server,
        const SecretCallback &secret_callback, 
        const DataCallback &data_callback)
    : ssl_(SSL_new(ctx.ctx_)),
      secret_callback_(secret_callback),
      data_callback_(data_callback) {

    if (is_server) {
        SSL_set_accept_state(ssl_);
    } else {
        SSL_set_connect_state(ssl_);
    }

    openssl_call("set_ex_data",
                 SSL_set_ex_data(ssl_, QUIC_EX_DATA_INDEX, this));

    openssl_call("set_min_protocol",
                 SSL_set_min_proto_version(ssl_, TLS1_3_VERSION));

    openssl_call("set_max_protocol",
                 SSL_set_max_proto_version(ssl_, TLS1_3_VERSION));

    openssl_call("set_quic_method",
                 SSL_set_quic_method(ssl_, &crypto::quic_method));

    // The session is thus considered to be shutdown, but no "close notify"
    // alert is sent to the peer. This behaviour violates the TLS standard.
    // TODO: SSL_set_quiet_shutdown ?
}

QuicTls::~QuicTls() {
    SSL_free(ssl_);
}

void QuicTls::set_transport_params(StringRef data) {
    openssl_call("set_quic_transport_params",
                 SSL_set_quic_transport_params(ssl_, data.data(), data.size()));
}

void QuicTls::provide_data(EncryptionLevel level, StringRef data) {
    openssl_call("quic_provide_data",
                 SSL_provide_quic_data(ssl_, (ssl_encryption_level_t) level,
                      data.data(), data.size()));
}

//
// Created by Chengke Wong on 2020/4/24.
//

#include "openssl/ssl.h"

#include "quic_tls.h"
#include "util/string.h"

using namespace crypto;

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

static int QUIC_EX_DATA_INDEX =
    SSL_get_ex_new_index(0, nullptr, nullptr, nullptr, nullptr);

QuicTls::QuicTls(QuicTlsContext ctx, bool is_server)
    : ssl_(SSL_new(ctx.ctx_)) {

    SSL_QUIC_METHOD quic_method{};

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
                 SSL_set_quic_method(ssl_, &quic_method));

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

// https://testssl.sh/openssl-iana.mapping.html

//
// Created by Chengke Wong on 2020/4/24.
//

/*
 * This file is not a part of the quic-lab. It serves as a reference to the
 * boringSSL API.
 *
 * https://commondatastorage.googleapis.com/chromium-boringssl-docs/ssl.h.html
 */

#ifdef __cplusplus
extern "C" {
#endif

// draft-ietf-quic-transport (section 7.4) defines the contents of a new TLS 
// extension (a TransportParameters struct) to unilaterally declare its 
// supported transport parameters.

# define TLSEXT_TYPE_quic_transport_parameters   0xffa5

/*
 * reference: https://github.com/openssl/openssl/pull/8797
 *
 * An implementation of the BoringSSL API. BoringSSL is developed by Google and
 * is a part of the Chromium QUIC stack. Note that BoringSSL changes a lot so
 * there are no guarantees of API or ABI stability.
 */


/* NOTE on SSL_do_handshake()
 *
 * When configured for QUIC, SSL_do_handshake() will drive the handshake as
 * before, but it will not use the configured BIO. It will call functions on
 * SSL_QUIC_METHOD to configure secrets and send data. 
 *
 * return SSL_ERROR_WANT_READ if data is needed from the peer.
 *
 * When received, the caller should call SSL_provide_quic_data() and then 
 * SSL_do_handshake() to continue the handshake.
 * After the handshake completes, the caller should call SSL_provide_quic_data()
 * for any post-handshake data, followed by SSL_process_quic_post_handshake() to
 * process it. 
 */

typedef enum ssl_encryption_level_t {
    // The initial encryption level that is used for client and server hellos.
    ssl_encryption_initial,
    // This is a write-level for the client and a read-level for the server.
    ssl_encryption_early_data,
    // The encryption level for the remainder of the handshake.
    ssl_encryption_handshake,
    // The encryption level for the application data.
    ssl_encryption_application,
} OSSL_ENCRYPTION_LEVEL;

typedef struct ssl_quic_method_st {
    // configures the read and write secrets for the given encryption level.
    // This function will always be called before an encryption level other 
    // than ssl_encryption_initial is used. Note, however, that secrets for 
    // a level may be configured before TLS is ready to send or accept data 
    // at that level.
    // 
    // This function should use SSL_get_current_cipher() to determine the TLS cipher suite.
    int (*set_encryption_secrets)(SSL *ssl, OSSL_ENCRYPTION_LEVEL level,
                                  const uint8_t *read_secret,
                                  const uint8_t *write_secret, size_t secret_len);

    // adds handshake data to the current flight at the given encryption level.
    int (*add_handshake_data)(SSL *ssl, OSSL_ENCRYPTION_LEVEL level,
                              const uint8_t *data, size_t len);

    // Callers should defer writing data to the network until flush_flight() to 
    // better pack QUIC packets into transport datagrams. Note a flight may contain 
    // data at several encryption levels.
    int (*flush_flight)(SSL *ssl);

    // sends a fatal alert at the specified encryption level.
    int (*send_alert)(SSL *ssl, enum ssl_encryption_level_t level, uint8_t alert);
} SSL_QUIC_METHOD;

int SSL_CTX_set_quic_method(SSL_CTX *ctx, const SSL_QUIC_METHOD *quic_method);
int SSL_set_quic_method(SSL *ssl, const SSL_QUIC_METHOD *quic_method);

int SSL_set_quic_transport_params(SSL *ssl,
                                  const uint8_t *params,
                                  size_t params_len);
void SSL_get_peer_quic_transport_params(const SSL *ssl,
                                        const uint8_t **out_params,
                                        size_t *out_params_len);

// To avoid DoS attacks, the QUIC implementation must limit the amount of data 
// being queued up. The implementation can call SSL_quic_max_handshake_flight_len() 
// to get the maximum buffer length at each encryption level.
size_t SSL_quic_max_handshake_flight_len(const SSL *ssl, OSSL_ENCRYPTION_LEVEL level);

OSSL_ENCRYPTION_LEVEL SSL_quic_read_level(const SSL *ssl);
OSSL_ENCRYPTION_LEVEL SSL_quic_write_level(const SSL *ssl);

// It is an error to call this function outside of the handshake or with 
// an encryption level other than the current read level. 
int SSL_provide_quic_data(SSL *ssl, OSSL_ENCRYPTION_LEVEL level,
                          const uint8_t *data, size_t len);

// This method processes any data that QUIC has provided after the handshake has 
// completed. This includes NewSessionTicket messages sent by the server.
int SSL_process_quic_post_handshake(SSL *ssl);

int SSL_is_quic(SSL *ssl);



#ifdef __cplusplus
}
#endif

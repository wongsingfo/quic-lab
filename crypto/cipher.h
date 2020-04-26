//
// Created by Chengke Wong on 2020/4/25.
//

#ifndef CRYPTO_CIPHER_H
#define CRYPTO_CIPHER_H

#include "util/string.h"
#include "crypto/aead.h"
#include "crypto/hp.h"
#include "crypto/hkdf.h"

// TLS 1.3 cipher suites
// https://www.rfc-editor.org/rfc/rfc8446.html#appendix-B.4
enum class CipherSuite {
    TLS_AES_128_GCM_SHA256 = 0x1301,
    TLS_AES_256_GCM_SHA384 = 0x1302,
    TLS_CHACHA20_POLY1305_SHA256 = 0x1303,
    TLS_AES_128_CCM_SHA256 = 0x1304,
// QUIC can use any of the ciphersuites defined in [TLS13] with the
// exception of TLS_AES_128_CCM_8_SHA256.
    // TLS_AES_128_CCM_8_SHA256 = 0x1305,
};

class Cipher {

public:

    Cipher(CipherSuite suite, StringRef secret);

    Cipher (Cipher&& other) = default;

    // |ikm| is the |dst_connection_id|
    static Cipher from_initial_secret(StringRef ikm, bool is_server);

    StringRef hp() const {
        return hp_;
    }

private:

    static AeadAlgorithm get_aead_algorithm(CipherSuite suite);

    static HkdfHash get_hkdf_hash(CipherSuite suite);

    static HpAlgorithm get_hp_algorithm(CipherSuite suite);

    static String derive_key(StringRef secret, const char *label, CipherSuite suite, size_t length);

    CipherSuite suite_;
    String key_;
    String iv_;
    String hp_;

    // disallow copy and assignment
    Cipher (const Cipher&) = delete;
    Cipher& operator = (const Cipher&) = delete;

};


#endif //CRYPTO_CIPHER_H

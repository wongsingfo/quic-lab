//
// Created by Chengke Wong on 2020/4/25.
//

#include "crypto/cipher.h"
#include "crypto/hkdf.h"

AeadAlgorithm Cipher::get_aead_algorithm(CipherSuite suite) {
    switch (suite) {
        case CipherSuite::TLS_AES_128_GCM_SHA256:
            return AeadAlgorithm::AEAD_AES_128_GCM;
        case CipherSuite::TLS_AES_256_GCM_SHA384:
            return AeadAlgorithm::AEAD_AES_256_GCM;
        case CipherSuite::TLS_CHACHA20_POLY1305_SHA256:
            return AeadAlgorithm::AEAD_CHACHA20_POLY1305;
        case CipherSuite::TLS_AES_128_CCM_SHA256:
            return AeadAlgorithm::AEAD_AES_128_CCM;
    }
}

HkdfHash Cipher::get_hkdf_hash(CipherSuite suite) {
    switch (suite) {
        case CipherSuite::TLS_AES_128_GCM_SHA256:
        case CipherSuite::TLS_AES_128_CCM_SHA256:
        case CipherSuite::TLS_CHACHA20_POLY1305_SHA256:
            return HkdfHash::SHA_256;
        case CipherSuite::TLS_AES_256_GCM_SHA384:
            return HkdfHash::SHA_384;
    }
}

String Cipher::derive_key(StringRef secret, const char *label,
                          CipherSuite suite) {
    return crypto::hkdf_expand_label(get_hkdf_hash(suite),
                                     secret,
                                     StringRef::from_text(label),
                                     StringRef::empty_string(),
                                     crypto::get_key_length(
                                         get_aead_algorithm(suite)));
}

// https://quicwg.org/base-drafts/draft-ietf-quic-tls.html#name-packet-protection-keys
Cipher::Cipher(CipherSuite suite, StringRef secret)
    : suite_(suite),
      key_(derive_key(secret, "quic key", suite)),
      iv_(derive_key(secret, "quic iv", suite)),
      hp_(derive_key(secret, "quic hp", suite)) {
}

Cipher Cipher::from_initial_secret(StringRef ikm, bool is_server) {
    static String initial_salt =
        String::from_hex("0xc3eef712c72ebb5a11a7d2432bb46365bef9f502");

    String initial_secret =
        crypto::hkdf_extract(HkdfHash::SHA_256, initial_salt, ikm);
    String secret =
        crypto::hkdf_expand_label(
            HkdfHash::SHA_256,
            initial_secret,
            StringRef::from_text(is_server ? "server in" : "client in"),
            StringRef::empty_string(),
            crypto::get_hash_length(HkdfHash::SHA_256));

    return Cipher(CipherSuite::TLS_AES_128_GCM_SHA256, secret);
}


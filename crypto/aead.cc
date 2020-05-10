//
// Created by Chengke Wong on 2020/4/9.
//

#include "crypto/aead.h"

#include "openssl/evp.h"
#include "openssl/aead.h"

#include "util/exception_ssl.h"

namespace crypto {

namespace openssl {

const EVP_AEAD *get_aead_algorithm(AeadAlgorithm aead) {
    switch (aead) {
        case AeadAlgorithm::AEAD_AES_128_GCM:
            return EVP_aead_aes_128_gcm();
        case AeadAlgorithm::AEAD_AES_256_GCM:
            return EVP_aead_aes_256_gcm();
        case AeadAlgorithm::AEAD_AES_128_CCM:
            return EVP_aead_aes_128_ccm_bluetooth();
        case AeadAlgorithm::AEAD_CHACHA20_POLY1305:
            return EVP_aead_chacha20_poly1305();
    }
}

// https://commondatastorage.googleapis.com/chromium-boringssl-docs/aead.h.html
void aead_inplace(const EVP_AEAD *aead, StringRef key,
    StringRef text, StringRef nonce, StringRef ad, size_t tag_len,
                  bool is_encrypt) {
    EVP_AEAD_CTX *ctx =
        EVP_AEAD_CTX_new(aead, key.data(), key.size(), tag_len);
    if (ctx == nullptr) {
        throw openssl_error("new ctx failed", 0);
    }

    size_t out_len;

    if (is_encrypt) {
        openssl_call("EVP_AEAD_CTX_seal",
                     EVP_AEAD_CTX_seal(ctx, text.data(), &out_len, text.size(),
                                       nonce.data(), nonce.size(),
                                       text.data(),
                                       text.size() - tag_len,
                                       ad.data(), ad.size()));
        DCHECK(out_len == text.size());
    } else {
        openssl_call("EVP_AEAD_CTX_open",
                     EVP_AEAD_CTX_open(ctx,
                                       text.data(), &out_len,
                                       text.size() - tag_len,
                                       nonce.data(), nonce.size(),
                                       text.data(), text.size(),
                                       ad.data(), ad.size()));
        DCHECK(out_len + tag_len == text.size());
    }

    EVP_AEAD_CTX_free(ctx);
}

} // namespace openssl

void aead_encrypt_inplace(AeadAlgorithm algo, StringRef key,
                          StringRef text, StringRef nonce, StringRef ad) {
    openssl::aead_inplace(openssl::get_aead_algorithm(algo),
                          key, text, nonce, ad,
                          get_tag_length(algo), true);
}

void aead_decrypt_inplace(AeadAlgorithm algo, StringRef key,
                          StringRef text, StringRef nonce, StringRef ad) {
    openssl::aead_inplace(openssl::get_aead_algorithm(algo),
                          key, text, nonce, ad,
                          get_tag_length(algo), false);
}


// The ciphersuites used by QUIC all have a 16-byte authentication tag and
// produce an output 16 bytes larger than their input.
size_t get_tag_length(AeadAlgorithm algo) {
    switch (algo) {
        case AeadAlgorithm::AEAD_AES_128_GCM:
            return 16;
        case AeadAlgorithm::AEAD_AES_256_GCM:
            return 16;
        case AeadAlgorithm::AEAD_AES_128_CCM:
            return 16;
        case AeadAlgorithm::AEAD_CHACHA20_POLY1305:
            return 16;
    }
}

size_t get_key_length(AeadAlgorithm algo) {
    switch (algo) {
        // https://www.rfc-editor.org/rfc/rfc5116.html#section-5
        case AeadAlgorithm::AEAD_AES_128_GCM:
            return 16;
        case AeadAlgorithm::AEAD_AES_256_GCM:
            return 32;
        case AeadAlgorithm::AEAD_AES_128_CCM:
            return 16;
        case AeadAlgorithm::AEAD_CHACHA20_POLY1305:
            // https://tools.ietf.org/html/rfc8103#section-1.1
            return 32;
    }
}

size_t get_iv_length(AeadAlgorithm algo) {
    switch (algo) {
        // https://www.rfc-editor.org/rfc/rfc5116.html#section-5
        case AeadAlgorithm::AEAD_AES_128_GCM:
            return 12;
        case AeadAlgorithm::AEAD_AES_256_GCM:
            return 12;
        case AeadAlgorithm::AEAD_AES_128_CCM:
            return 12;
        case AeadAlgorithm::AEAD_CHACHA20_POLY1305:
            // https://tools.ietf.org/html/rfc8103#section-1.1
            return 12;
    }
}

} // namespace crypto

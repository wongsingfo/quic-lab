//
// Created by Chengke Wong on 2020/4/9.
//

#include "crypto/aead.h"

#include "openssl/evp.h"
#include "openssl/aead.h"

#include "exception.h"

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
    StringRef text, StringRef nonce, StringRef ad,
                  bool is_encrypt) {
    EVP_AEAD_CTX *ctx =
        EVP_AEAD_CTX_new(aead, key.data(), key.size(), QUIC_AEAD_TAG_LENGTH);
    if (ctx == nullptr) {
        throw openssl_error("new ctx failed", 0);
    }

    size_t out_len;

    if (is_encrypt) {
        openssl_call("EVP_AEAD_CTX_seal",
                     EVP_AEAD_CTX_seal(ctx, text.data(), &out_len, text.size(),
                                       nonce.data(), nonce.size(),
                                       text.data(),
                                       text.size() - QUIC_AEAD_TAG_LENGTH,
                                       ad.data(), ad.size()));
        dynamic_check(out_len == text.size());
    } else {
        openssl_call("EVP_AEAD_CTX_open",
                     EVP_AEAD_CTX_open(ctx,
                                       text.data(), &out_len,
                                       text.size() - QUIC_AEAD_TAG_LENGTH,
                                       nonce.data(), nonce.size(),
                                       text.data(), text.size(),
                                       ad.data(), ad.size()));
        dynamic_check(out_len + QUIC_AEAD_TAG_LENGTH == text.size());
    }

    EVP_AEAD_CTX_free(ctx);
}

} // namespace openssl

void aead_encrypt_inplace(AeadAlgorithm algo, StringRef key,
                          StringRef text, StringRef nonce, StringRef ad) {
    openssl::aead_inplace(openssl::get_aead_algorithm(algo),
                          key, text, nonce, ad, true);
}

void aead_decrypt_inplace(AeadAlgorithm algo, StringRef key,
                          StringRef text, StringRef nonce, StringRef ad) {
    openssl::aead_inplace(openssl::get_aead_algorithm(algo),
                          key, text, nonce, ad, false);
}

} // namespace crypto

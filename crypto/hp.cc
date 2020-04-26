//
// Created by Chengke Wong on 2020/4/25.
//

#include "crypto/hp.h"

#include "openssl/ossl_typ.h"
#include "openssl/evp.h"
#include "openssl/chacha.h"

namespace crypto {

namespace openssl {

static String aes_ecb(const EVP_CIPHER *(*method)(),
                      StringRef key, StringRef text,
                      bool is_encrypt) {
    if (text.size() != 16) {
        throw std::invalid_argument("text length should be of 16");
    }

    int x = text[0];

    EVP_CIPHER_CTX *ctx;
    ctx = EVP_CIPHER_CTX_new();
    openssl_call("EVP_CipherInit_ex",
                 EVP_CipherInit_ex(ctx,
                                   method(), nullptr,
                                   key.data(), nullptr, is_encrypt));

    int out_len;
    String out(16);
    openssl_call("EVP_CipherUpdate",
                 EVP_CipherUpdate(ctx, out.data(), &out_len,
                                  text.data(), text.size()));
    EVP_CIPHER_CTX_free(ctx);
    assert(x == text[0]);

    return out;
}

// https://www.rfc-editor.org/rfc/rfc8439.html#section-2.4.1
// A ChaCha20 implementation could take a 32-bit integer in place of a byte
// sequence, in which case the byte sequence is interpreted as a little-endian
// value.
void chacha20_inplace(StringRef key, uint32_t counter,
                StringRef nonce, StringRef text) {
    if (key.size() != 32) {
        throw std::invalid_argument("key length should be of 32");
    }
    if (nonce.size() != 12) {
        throw std::invalid_argument("nonce length should be of 12");
    }
    CRYPTO_chacha_20(text.data(), text.data(), text.size(),
                     key.data(), nonce.data(), counter);
}

} // namespace openssl

String aes_128_ecb_decrypt(StringRef key, StringRef ciphertext) {
    if (key.size() != 16) {
        throw std::invalid_argument("key length should be of 16");
    }
    return openssl::aes_ecb(EVP_aes_128_ecb, key, ciphertext, false);
}

String aes_128_ecb_encrypt(StringRef key, StringRef plaintext) {
    if (key.size() != 16) {
        throw std::invalid_argument("key length should be of 16");
    }
    return openssl::aes_ecb(EVP_aes_128_ecb, key, plaintext, true);
}

String aes_256_ecb_encrypt(StringRef key, StringRef plaintext) {
    if (key.size() != 32) {
        throw std::invalid_argument("key length should be of 32");
    }
    return openssl::aes_ecb(EVP_aes_256_ecb, key, plaintext, true);
}

static String chacha_20_encrypt_decrypt(StringRef hp_key, StringRef sample) {
    // counter = sample[0..3]    (interpreted as little endian)
    // nonce = sample[4..15]
    // mask = ChaCha20(hp_key, counter, nonce, {0,0,0,0,0})
    String result(5);
    uint32_t counter = sample[0];
    counter |= sample[1] << 8;
    counter |= sample[2] << 16;
    counter |= sample[3] << 24;
    openssl::chacha20_inplace(hp_key, counter, sample.sub_string(4, 15), result);
    return result;
}

size_t get_hp_key_length(HpAlgorithm algo) {
    switch (algo) {
        case HpAlgorithm::AES_ECB_128:
            return 16;
        case HpAlgorithm::AES_ECB_256:
            return 32;
        case HpAlgorithm::ChaCha_20:
            return 32;
    }
}

String get_hp_mask(HpAlgorithm algo, StringRef hp_key, StringRef sample) {
    switch (algo) {
        case HpAlgorithm::AES_ECB_128:
            return aes_128_ecb_encrypt(hp_key, sample);
        case HpAlgorithm::AES_ECB_256:
            return aes_256_ecb_encrypt(hp_key, sample);
        case HpAlgorithm::ChaCha_20:
            return chacha_20_encrypt_decrypt(hp_key, sample);
    }
}

} // namespace crypto

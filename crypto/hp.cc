//
// Created by Chengke Wong on 2020/4/25.
//

#include "crypto/hp.h"

#include "openssl/ossl_typ.h"
#include "openssl/evp.h"

namespace crypto {

namespace openssl {

static String aes_ecb(const EVP_CIPHER *(*method)(),
                      StringRef key, StringRef text,
                      bool is_encrypt) {
    if (key.size() != 16) {
        throw std::invalid_argument("key length should be of 16");
    }
    if (text.size() != 16) {
        throw std::invalid_argument("text length should be of 16");
    }

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

    return out;
}

} // namespace openssl

String aes_128_ecb_decrypt(StringRef key, StringRef ciphertext) {
    return openssl::aes_ecb(EVP_aes_128_ecb, key, ciphertext, false);
}

String aes_128_ecb_encrypt(StringRef key, StringRef plaintext) {
    return openssl::aes_ecb(EVP_aes_128_ecb, key, plaintext, true);
}

String get_hp_mask(HpAlgorithm algo, StringRef sample) {

}

} // namespace crypto

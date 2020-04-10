//
// Created by Chengke Wong on 2020/4/9.
//

#include "crypto/aead.h"

#include <openssl/ossl_typ.h>
#include <openssl/evp.h>

#include "exception.h"

namespace crypto {

namespace openssl {

static String aes_ecb(const EVP_CIPHER *(*method)(),
                      StringRef key, StringRef test,
                      bool is_encrypt) {
    if (key.size() != 16) {
        throw std::invalid_argument("key length should be of 16");
    }
    if (test.size() != 16) {
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
    // https://www.openssl.org/docs/man1.1.0/man3/EVP_aes_128_cbc.html
    // After this function is called the encryption operation is finished
    // and no further calls to EVP_EncryptUpdate() should be made.
    openssl_call("EVP_CipherUpdate",
                 EVP_CipherUpdate(ctx, out.data(), &out_len,
                                  test.data(), test.size()));
    EVP_CIPHER_CTX_free(ctx);

    return out;
}

} // namespace openssl

String aes_128_ecb_decrypt(StringRef key, StringRef ciphertext) {
    return openssl::aes_ecb(EVP_aes_128_ecb, key, ciphertext, false);
}

String aes_128_ecb_encrypt(StringRef key, StringRef ciphertext) {
    return openssl::aes_ecb(EVP_aes_128_ecb, key, ciphertext, true);
}

} // namespace crypto

//
// Created by Chengke Wong on 2020/4/9.
//

#include "crypto/aead.h"

#include <openssl/ossl_typ.h>
#include <openssl/evp.h>

#include "exception.h"

namespace crypto {

namespace openssl {

void aes_128_gcm(StringRef key, StringRef aad,
                   StringRef nonce,
                   StringRef text,
                   StringRef tag,
                   StringRef output,
                   bool is_encrypt) {
    if (nonce.size() != 12) {
        throw std::invalid_argument("the iv length must be 12 bytes");
    }
    if (key.size() != 16) {
        throw std::invalid_argument("key length should be of 16");
    }
    if (tag.size() != 16) {
        throw std::invalid_argument("tag length should be of 16");
    }
    if (text.size() != output.size()) {
        throw std::invalid_argument("the text lengths differ");
    }

    int len;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    openssl_call("Initialise the decryption operation",
                 EVP_CipherInit_ex(ctx, EVP_aes_128_gcm(), nullptr, nullptr, nullptr, is_encrypt));
    openssl_call("Initialise key and IV",
                 EVP_CipherInit_ex(ctx, nullptr, nullptr, key.data(), nonce.data(), is_encrypt));
    openssl_call("Provide any AAD data",
                 EVP_CipherUpdate(ctx, nullptr, &len, aad.data(), aad.size()));

    openssl_call("EVP_CipherUpdate",
                 EVP_CipherUpdate(ctx, output.data(), &len,
                                   text.data(), text.size()));
    if (! is_encrypt) {
        openssl_call("set the tag",
                     EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG,
                                         tag.size(), tag.data()));
    }

    openssl_call("check the 16-byte tag",
                 EVP_CipherFinal_ex(ctx, output.data() + len, &len));

    if (is_encrypt) {
        openssl_call("set the tag",
                     EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG,
                                         tag.size(), tag.data()));
    }

    EVP_CIPHER_CTX_free(ctx);
}

} // namespace openssl

String aes_128_gcm_encrypt(StringRef key, StringRef aad,
                           StringRef nonce,
                           StringRef plaintext) {
    String output(plaintext.size() + 16);
    openssl::aes_128_gcm(key, aad, nonce,
                         plaintext,
                         output.sub_string(plaintext.size()),
                         output.sub_string(0, plaintext.size()),
                         true);
    return output;
}

String aes_128_gcm_decrypt(StringRef key, StringRef aad,
                           StringRef nonce,
                           StringRef ciphertext) {
    String output(ciphertext.size() - 16);
    openssl::aes_128_gcm(key, aad, nonce,
                         ciphertext.sub_string(0, ciphertext.size() - 16),
                         ciphertext.sub_string(ciphertext.size() - 16),
                         output,
                         false);
    return output;
}

} // namespace crypto

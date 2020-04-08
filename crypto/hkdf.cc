#include "crypto/hkdf.h"

/* OpenSSL version: 1.1.0 */
#include <openssl/kdf.h>
#include <openssl/evp.h>

namespace crypto {

String hkdf_expand(String salt, String secret, String label, int length) {
    unsigned char output[length];
    EVP_PKEY_CTX *pctx;

    pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
    openssl_call("init", EVP_PKEY_derive_init(pctx));
    openssl_call("set mode", EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()));
    openssl_call("add salt", 
        EVP_PKEY_CTX_set1_hkdf_salt(pctx, salt.data(), salt.size()));
    openssl_call("add secret",
        EVP_PKEY_CTX_set1_hkdf_key(pctx, secret.data(), secret.size()));
    openssl_call("add label",
        EVP_PKEY_CTX_add1_hkdf_info(pctx, label.data(), label.size()));

    size_t rv = length;
    openssl_call("derive key",
        EVP_PKEY_derive(pctx, output, &rv));
    if (rv != length) {
        throw std::runtime_error("unmatched output");
    }

    return String(output, rv);
}

}

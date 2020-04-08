#include "crypto/hkdf.h"

/* OpenSSL version: 1.1.0 */
#include <openssl/kdf.h>
#include <openssl/evp.h>

#include "util/string_writer.h"

namespace crypto {

String hkdf(const String &salt,
            const String &secret,
            const String &info,
            size_t length) {
    // https://www.openssl.org/docs/man1.1.0/man3/EVP_PKEY_CTX_set_hkdf_md.html

    unsigned char output[length];
    EVP_PKEY_CTX *pctx;

    pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr);
    openssl_call("init", EVP_PKEY_derive_init(pctx));
    openssl_call("set mode", EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()));
    openssl_call("add salt", 
        EVP_PKEY_CTX_set1_hkdf_salt(pctx, salt.data(), salt.size()));
    openssl_call("add secret",
        EVP_PKEY_CTX_set1_hkdf_key(pctx, secret.data(), secret.size()));
    openssl_call("add label",
        EVP_PKEY_CTX_add1_hkdf_info(pctx, info.data(), info.size()));

    size_t rv = length;
    openssl_call("derive key",
        EVP_PKEY_derive(pctx, output, &rv));

    return String(output, rv);
}

String hkdf_label(const String &salt, const String &secret, const String &label,
                  size_t length) {
    constexpr const char* prepend = "tls13 ";
    StringWriter info(sizeof(uint16_t) +
                      1 + strlen(prepend) + label.size() +
                      1); //
    info.write_u16(length);
    info.write_u8(strlen(prepend) + label.size());
    info.write(prepend);
    info.write(label);
    info.write_u8(0);
    return hkdf(salt, secret, info, length);
}

}

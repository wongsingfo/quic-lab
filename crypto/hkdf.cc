#include "crypto/hkdf.h"

/* OpenSSL version: 1.1.0 */
#include <openssl/hkdf.h>
#include <openssl/mem.h>
#include <openssl/hmac.h>
#include <openssl/err.h>

#include "util/string_writer.h"

namespace crypto {

namespace openssl {


// adapted from https://github.com/openssl/openssl/blob/a76ce2862bc6ae2cf8a749c8747d371041fc42d1/providers/implementations/kdfs/hkdf.c

/*
 * Refer to "HMAC-based Extract-and-Expand Key Derivation Function (HKDF)"
 * Section 2.2 (https://tools.ietf.org/html/rfc5869#section-2.2).
 *
 * 2.2.  Step 1: Extract
 *
 *   HKDF-Extract(salt, IKM) -> PRK
 *
 *   Options:
 *      Hash     a hash function; HashLen denotes the length of the
 *               hash function output in octets
 *
 *   Inputs:
 *      salt     optional salt value (a non-secret random value);
 *               if not provided, it is set to a string of HashLen zeros.
 *      IKM      input keying material
 *
 *   Output:
 *      PRK      a pseudorandom key (of HashLen octets)
 *
 *   The output PRK is calculated as follows:
 *
 *   PRK = HMAC-Hash(salt, IKM)
 */

static void HKDF_Extract(const EVP_MD *evp_md,
                        const unsigned char *salt, size_t salt_len,
                        const unsigned char *ikm, size_t ikm_len,
                        unsigned char *prk, size_t prk_len)
{
    int sz = EVP_MD_size(evp_md);

    if (sz < 0)
        throw std::invalid_argument("sz");
    if (prk_len != (size_t)sz) {
        throw std::invalid_argument("wrong output buffer size");
    }
    /* calc: PRK = HMAC-Hash(salt, IKM) */
    if (HMAC(evp_md, salt, salt_len, ikm, ikm_len, prk, nullptr) == nullptr) {
        throw openssl_error("HMAC", 0);
    }
}

/*
 * Refer to "HMAC-based Extract-and-Expand Key Derivation Function (HKDF)"
 * Section 2.3 (https://tools.ietf.org/html/rfc5869#section-2.3).
 *
 * 2.3.  Step 2: Expand
 *
 *   HKDF-Expand(PRK, info, L) -> OKM
 *
 *   Options:
 *      Hash     a hash function; HashLen denotes the length of the
 *               hash function output in octets
 *
 *   Inputs:
 *      PRK      a pseudorandom key of at least HashLen octets
 *               (usually, the output from the extract step)
 *      info     optional context and application specific information
 *               (can be a zero-length string)
 *      L        length of output keying material in octets
 *               (<= 255*HashLen)
 *
 *   Output:
 *      OKM      output keying material (of L octets)
 *
 *   The output OKM is calculated as follows:
 *
 *   N = ceil(L/HashLen)
 *   T = T(1) | T(2) | T(3) | ... | T(N)
 *   OKM = first L octets of T
 *
 *   where:
 *   T(0) = empty string (zero length)
 *   T(1) = HMAC-Hash(PRK, T(0) | info | 0x01)
 *   T(2) = HMAC-Hash(PRK, T(1) | info | 0x02)
 *   T(3) = HMAC-Hash(PRK, T(2) | info | 0x03)
 *   ...
 *
 *   (where the constant concatenated to the end of each T(n) is a
 *   single octet.)
 */
static void HKDF_Expand(const EVP_MD *evp_md,
                       const unsigned char *prk, size_t prk_len,
                       const unsigned char *info, size_t info_len,
                       unsigned char *okm, size_t okm_len) {
    HMAC_CTX *hmac;
    int ret = 0, sz;
    unsigned int i;
    unsigned char prev[EVP_MAX_MD_SIZE];
    size_t done_len = 0, dig_len, n;

    sz = EVP_MD_size(evp_md);
    if (sz <= 0)
        throw std::invalid_argument("sz");
    dig_len = (size_t) sz;

    /* calc: N = ceil(L/HashLen) */
    n = okm_len / dig_len;
    if (okm_len % dig_len)
        n++;

    if (n > 255 || okm == nullptr)
        throw std::invalid_argument("okm");

    if ((hmac = HMAC_CTX_new()) == nullptr)
        throw std::invalid_argument("hmac");

    if (!HMAC_Init_ex(hmac, prk, prk_len, evp_md, nullptr))
        goto err;

    for (i = 1; i <= n; i++) {
        size_t copy_len;
        const unsigned char ctr = i;

        /* calc: T(i) = HMAC-Hash(PRK, T(i - 1) | info | i) */
        if (i > 1) {
            if (!HMAC_Init_ex(hmac, nullptr, 0, nullptr, nullptr))
                goto err;

            if (!HMAC_Update(hmac, prev, dig_len))
                goto err;
        }

        if (!HMAC_Update(hmac, info, info_len))
            goto err;

        if (!HMAC_Update(hmac, &ctr, 1))
            goto err;

        if (!HMAC_Final(hmac, prev, nullptr))
            goto err;

        copy_len = (done_len + dig_len > okm_len) ?
                   okm_len - done_len :
                   dig_len;

        memcpy(okm + done_len, prev, copy_len);

        done_len += copy_len;
    }
    ret = 1;

    err:
    OPENSSL_cleanse(prev, sizeof(prev));
    HMAC_CTX_free(hmac);

    if (ret == 0) {
        throw openssl_error("hkdf_expand", ret);
    }
}

/*
 * Refer to "HMAC-based Extract-and-Expand Key Derivation Function (HKDF)"
 * Section 2 (https://tools.ietf.org/html/rfc5869#section-2) and
 * "Cryptographic Extraction and Key Derivation: The HKDF Scheme"
 * Section 4.2 (https://eprint.iacr.org/2010/264.pdf).
 *
 * From the paper:
 *   The scheme HKDF is specified as:
 *     HKDF(XTS, SKM, CTXinfo, L) = K(1) | K(2) | ... | K(t)
 *
 *     where:
 *       SKM is source key material
 *       XTS is extractor salt (which may be null or constant)
 *       CTXinfo is context information (may be null)
 *       L is the number of key bits to be produced by KDF
 *       k is the output length in bits of the hash function used with HMAC
 *       t = ceil(L/k)
 *       the value K(t) is truncated to its first d = L mod k bits.
 *
 * From RFC 5869:
 *   2.2.  Step 1: Extract
 *     HKDF-Extract(salt, IKM) -> PRK
 *   2.3.  Step 2: Expand
 *     HKDF-Expand(PRK, info, L) -> OKM
 */
static void HKDF(const EVP_MD *evp_md,
                const unsigned char *salt, size_t salt_len,
                const unsigned char *ikm, size_t ikm_len,
                const unsigned char *info, size_t info_len,
                unsigned char *okm, size_t okm_len)
{
    unsigned char prk[EVP_MAX_MD_SIZE];
    int ret, sz;
    size_t prk_len;

    sz = EVP_MD_size(evp_md);
    if (sz < 0)
        throw std::invalid_argument("sz");
    prk_len = (size_t)sz;

    /* Step 1: HKDF-Extract(salt, IKM) -> PRK */
    HKDF_Extract(evp_md, salt, salt_len, ikm, ikm_len, prk, prk_len);
    /* Step 2: HKDF-Expand(PRK, info, L) -> OKM */
    HKDF_Expand(evp_md, prk, prk_len, info, info_len, okm, okm_len);

    OPENSSL_cleanse(prk, sizeof(prk));
}

} // namespace openssl

const EVP_MD* get_hash_method(HkdfHash hash) {
    switch (hash) {
        case HkdfHash::SHA_256:
            return EVP_sha256();
        case HkdfHash::SHA_384:
            return EVP_sha384();
    }
}


String hkdf(HkdfHash hash, StringRef salt, StringRef secret, StringRef info,
            size_t length) {
    // https://www.openssl.org/docs/man1.1.0/man3/EVP_PKEY_CTX_set_hkdf_md.html

    String output(length);

    openssl::HKDF(get_hash_method(hash),
                  salt.data(), salt.size(),
                  secret.data(), secret.size(),
                  info.data(), info.size(),
                  output.data(), output.size());


    return output;
}

String hkdf_label(const StringRef salt,
                  const StringRef secret,
                  const StringRef label,
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
    return hkdf(HkdfHash::SHA_256, salt, secret, info, length);
}

String hkdf_extract(HkdfHash hash, StringRef salt, StringRef ikm) {
    const env_md_st *method = get_hash_method(hash);
    String result(EVP_MD_size(method));
    openssl::HKDF_Extract(method,
                          salt.data(), salt.size(),
                          ikm.data(), ikm.size(),
                          result.data(), result.size());
    return result;
}

String hkdf_expand(HkdfHash hash, const StringRef prk,
                   const StringRef info, size_t length) {
    String result(length);
    openssl::HKDF_Expand(get_hash_method(hash),
                         prk.data(), prk.size(),
                         info.data(), info.size(),
                         result.data(), result.size());
    return result;
}

String hkdf_expand_label(HkdfHash hash,
                         const StringRef prk,
                         const StringRef label,
                         const StringRef context,
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

    return hkdf_expand(hash, prk, info, length);
}


} // namespace crypto


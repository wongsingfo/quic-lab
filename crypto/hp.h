//
// Created by Chengke Wong on 2020/4/25.
//

#ifndef CRYPTO_HP_H
#define CRYPTO_HP_H

#include "util/string.h"
#include "crypto/exception.h"

namespace crypto {

enum class HpAlgorithm {
// https://quicwg.org/base-drafts/draft-ietf-quic-tls.html#name-aes-based-header-protection
// ADVANCED ENCRYPTION STANDARD:
//   https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.197.pdf
    AES_ECB_128,
    AES_ECB_256,

// https://quicwg.org/base-drafts/draft-ietf-quic-tls.html#name-chacha20-based-header-prote
    ChaCha_20,
};

String get_hp_mask(HpAlgorithm algo, StringRef sample);

String aes_128_ecb_encrypt(StringRef key, StringRef plaintext);

String aes_128_ecb_decrypt(StringRef key, StringRef ciphertext);


} // namespace crypto

#endif //CRYPTO_HP_H

//
// Created by Chengke Wong on 2020/4/9.
//

#ifndef CRYPTO_AEAD_H
#define CRYPTO_AEAD_H

#include "util/string.h"

namespace crypto {

// These ciphersuites have a 16-byte authentication tag and produce an output
// 16 bytes larger than their input.
enum class AeadAlgorithm {
    // conform to the IANA Consideration
    AEAD_AES_128_GCM = 1,
    AEAD_AES_256_GCM = 2,
    AEAD_AES_128_CCM = 3,
    AEAD_AES_256_CCM = 4,
};

// https://www.rfc-editor.org/rfc/inline-errata/rfc5116.html

/*
 * The authenticated encryption operation has four inputs, each of which is an
 * octet string:
 *
 *    A secret key K, which MUST be generated in a way that is uniformly
 *    random or pseudorandom.
 *    K_LEN:
 *       AEAD_AES_128_GCM = 16
 *
 *    A nonce N.  Each nonce provided to distinct invocations of the
 *    Authenticated Encryption operation MUST be distinct, for any
 *    particular value of the key, unless each and every nonce is zero-
 *    length.
 *
 *    A plaintext P, which contains the data to be encrypted and
 *    authenticated.
 *    Length: [0, P_MAX], P_MAX >= 2^16
 *
 *    The associated data A, which contains the data to be authenticated, 
 *    but not encrypted.
 *    Length: [N_MIN, N_MAX], N_MIN <= 12 <= N_MAX
 *
 * There is a single output:
 *
 *    A ciphertext C, which is at least as long as the plaintext
 *    Length: [0, A_MAX];
 */

// https://wiki.openssl.org/index.php/EVP_Authenticated_Encryption_and_Decryption

String aes_128_ecb_encrypt(StringRef key, StringRef ciphertext);

/*
 * The authenticated decryption operation has four inputs: K, N, A, and C,
 * as defined above.
 *
 * It will, with high probability, return FAIL whenever the inputs N, C, and A
 * were crafted by a nonce-respecting adversary that does not know the secret
 * key.
 */

String aes_128_ecb_decrypt(StringRef key, StringRef ciphertext);

/* Packet Protection
 * https://quicwg.org/base-drafts/draft-ietf-quic-tls.html#name-aead-usage
 *
 * secret = current encryption level secret
 * K  = HKDF_Expand(secret, "quic key", _, 16)
 * IV = HKDF_Expand(secret, "quic iv", _, 12)
 * N  = IV xor packet_number (left-padded with zero)
 * A  = starting from the flags bytes up to and including the unprotected
 *      packet number
 * P  = the payload of the packet
 */

/* Packet Header Protection
 *   This process does not apply to Retry or Version Negotiation packets
 * https://quicwg.org/base-drafts/draft-ietf-quic-tls.html#name-header-protection
 *
 * hp_key = HKDF_Expand(secret, "quic hp", _, 16)
 *
 * Prior to TLS selecting a ciphersuite, AES header protection is used,
 * matching the AEAD_AES_128_GCM packet protection.
 *
 * sample = starting from the payload (the Packet Number field is
            assumed to be 4 bytes long) up to 16 bytes
 * mask   = AES-ECB(hp_key, sample)
 *   - AEAD_AES_128_GCM and AEAD_AES_128_CCM use 128-bit AES in electronic code-book (ECB) mode.
 *   - AEAD_AES_256_GCM uses 256-bit AES in ECB mode.
 *
 * counter= sample[0..3]
 * nounce = sample[4..15]
 * mask   = ChaCha20(hp_key, counter, nonce, {0,0,0,0,0})
 */

} // namespace crypto

#endif //CRYPTO_AEAD_H

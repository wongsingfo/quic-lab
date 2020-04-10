// adapted from https://github.com/massar/rfc6234

#ifndef HKDF_H
#define HKDF_H

#include "util/utility.h"
#include "util/exception.h"
#include "util/string.h"
#include "crypto/exception.h"

namespace crypto {

/* https://www.rfc-editor.org/rfc/rfc5869
 *  (with test cases)
 *
 * Step 1: Extract
 *   HKDF-Extract(salt, IKM) -> PRK
 *   where
 *     salt: a non-secret random value
 *     IKM: input keying material
 *     PRK: a pseudorandom key
 *
 * Step 2: Expand
 *   HKDF-Expand(PRK, info, L) -> OKM
 *   where
 *     info: a pseudorandom key of at least HashLen octets
 *     L: length of output
 *     OKM: output key material ( <= 255 * HashLen )
 *     HashLen: the length of the hash function output in octets
 */


String hkdf_extract(StringRef salt,
                    StringRef ikm);

String hkdf_expand(StringRef prk, StringRef info, size_t length);

// a wrapper function hkdf_extract() and hkdf_expand()
String hkdf(StringRef salt,
            StringRef secret,
            StringRef info,
            size_t length);

/*
 * https://www.rfc-editor.org/rfc/rfc8446.html
 * Section 3.4
 *
 * opaque mandatory<300..400>;
 *   length field is two bytes, cannot be empty
 *
 * Section 7.1
 *
 * The `info` in Step 2 change to the concatenation of
 *   1. uint16 length = L
 *   2. opaque label<7..255> = "tls13 " + Label
 *   3. opaque context<0..255> = Context
 *
 *  (Single-byte entities containing uninterpreted data are of type opaque.)
 *
 * In QUIC, the `context` is always empty.
 */

String hkdf_expand_label(StringRef prk,
                         StringRef label,
                         StringRef context,
                         size_t length);

// a wrapper function for hkdf_extract() and hkdf_expand_label()
String hkdf_label(StringRef salt,
                  StringRef secret,
                  StringRef label,
                  size_t length);

}

#endif

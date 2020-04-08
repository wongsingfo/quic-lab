// adapted from https://github.com/massar/rfc6234

#ifndef HKDF_H
#define HKDF_H 

#include "util/utility.h"
#include "util/exception.h"
#include "util/string.h"
#include "crypto/exception.h"

namespace crypto {

/*
 * Description:
 *     using an HMAC hash function SHA-256 to expend the `prk`
 */
String hkdf_expand(String salt, String secret, String label, int length);

}

#endif

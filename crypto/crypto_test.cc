#include "gtest/gtest.h"

#include "crypto/hkdf.h"
#include "crypto/aead.h"
#include "crypto/hp.h"
#include "crypto/cipher.h"

namespace crypto {

class CryptoTest : public ::testing::Test {
protected:
    CryptoTest() = default;
    ~CryptoTest() override = default;

    void SetUp() override {
    }

    void TearDown() override {
    };

    String initial_salt =
        String::from_hex("0xc3eef712c72ebb5a11a7d2432bb46365bef9f502");

    /* The Destination Connection ID is of arbitrary length, and it 
     * could be zero length if the server sends a Retry packet with 
     * a zero-length Source Connection ID field. */
    String DCID =
        String::from_hex("0x8394c8f03e515708");

    String client_initial_secret =
        String::from_hex("fda3953aecc040e48b34e27ef87de3a6"
                         "098ecf0e38b7e032c5c57bcbd5975b84");

    String client_hp =
        String::from_hex("a980b8b4fb7d9fbc13e814c23164253d");
};

TEST_F(CryptoTest, Test01) {
    String output =
        hkdf(HkdfHash::SHA_256, String::from_hex("0x000102030405060708090a0b0c"),
             String::from_hex("0x0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b"),
             String::from_hex("0xf0f1f2f3f4f5f6f7f8f9"),
             42);

    EXPECT_EQ(output.to_hex(),
              "3cb25f25faacd57a90434f64d0362f2a"
              "2d2d0a90cf1a5a4c5db02d56ecc4c5bf"
              "34007208d5b887185865");
}

TEST_F(CryptoTest, ClientInSecret) {

    String output =
        hkdf_label(initial_salt,
                   DCID,
                   String::from_text("client in"),
                   32);

    EXPECT_EQ(output.size(), 32);

    EXPECT_EQ(output.to_hex(),
              client_initial_secret.to_hex());
}

TEST_F(CryptoTest, ServerInitialHP) {
    String server_initial_secret =
        hkdf_label(initial_salt,
                   DCID,
                   String::from_text("server in"),
                   32);

    EXPECT_EQ(server_initial_secret.to_hex(),
              "554366b81912ff90be41f17e80222130"
              "90ab17d8149179bcadf222f29ff2ddd5");

    String output =
        hkdf_expand_label(HkdfHash::SHA_256,
                          server_initial_secret,
                          String::from_text("quic hp"),
                          StringRef::empty_string(),
                          16);

    EXPECT_EQ(output.size(), 16);

    EXPECT_EQ(output.to_hex(),
              "a8ed82e6664f865aedf6106943f95fb8");
}

TEST_F(CryptoTest, ClientHPSecret) {
    String output =
        hkdf_expand_label(HkdfHash::SHA_256, client_initial_secret,
                          String::from_text("quic hp"),
                          String::empty_string(),
                          16);

    EXPECT_EQ(output.size(), 16);

    EXPECT_EQ(output,
              client_hp);
}

// https://github.com/openssl/openssl/blob/f7382fbbd846dd3bdea6b8c03b6af22faf0ab94f/test/recipes/30-test_evp_data/evpciph.txt
TEST_F(CryptoTest, Cipher_AES_128_ECB) {
    String key = String::from_hex("2B7E151628AED2A6ABF7158809CF4F3C");
    String plain = String::from_hex("F69F2445DF4F9B17AD2B417BE66C3710");
    String cipher = String::from_hex("7B0C785E27E8AD3F8223207104725DD4");
    EXPECT_EQ(aes_128_ecb_decrypt(key, cipher), plain);
    EXPECT_EQ(aes_128_ecb_encrypt(key, plain), cipher);
}

TEST_F(CryptoTest, Cipher_AES_256_ECB) {
    String key = String::from_hex("000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F");
    String plain = String::from_hex("00112233445566778899AABBCCDDEEFF");
    String cipher = String::from_hex("8EA2B7CA516745BFEAFC49904B496089");
    EXPECT_EQ(aes_256_ecb_encrypt(key, plain), cipher);
}

TEST_F(CryptoTest, ChaCha20) {
    String key = String::from_hex("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f");
    String nonce = String::from_hex("000000000000004a00000000");
    String text = StringRef::from_text("Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.").clone();
    openssl::chacha20_inplace(key, 1, nonce, text);
    EXPECT_EQ(text.sub_string(0, 16),
              String::from_hex("6e2e359a2568f980 41ba0728dd0d6981"));
    openssl::chacha20_inplace(key, 1, nonce, text);
    EXPECT_EQ(text.sub_string(0, 6),
              String::from_text("Ladies"));
}

TEST_F(CryptoTest, Cipher_AES_128_GCM) {
    String key = String::from_hex("feffe9928665731c6d6a8f9467308308");
    String plain = String::from_hex(
        "d9313225f88406e5a55909c5aff5269a86a7a9531534f7da2e4c303d8a318a721c3c0c95956809532fcf0e2449a6b525b16aedf5aa0de657ba637b39"
        "00000000000000000000000000000000");
    String cipher = String::from_hex(
        "42831ec2217774244b7221b784d0d49ce3aa212f2c02a4e035c17e2329aca12e21d514b25466931c7d8f6a5aac84aa051ba30b396a0aac973d58e091"
        "5bc94fbc3221a5db94fae95ae7121a47");
    String AAD = String::from_hex("feedfacedeadbeeffeedfacedeadbeefabaddad2");
    String nonce = String::from_hex("cafebabefacedbaddecaf888");
    aead_encrypt_inplace(AeadAlgorithm::AEAD_AES_128_GCM,
                         key, plain, nonce, AAD);
    EXPECT_EQ(plain, cipher);
}

TEST_F(CryptoTest, CipherSuiteInitial) {
    Cipher cipher = Cipher::from_initial_secret(DCID, false);
    EXPECT_EQ(cipher.hp(), client_hp);
}

} // namespace crypto

#include "gtest/gtest.h"

#include "hkdf.h"
#include "aead.h"

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
        hkdf(String::from_hex("0x000102030405060708090a0b0c"),
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
    String initial_salt_server =
        hkdf_label(initial_salt,
                   DCID,
                   String::from_text("server in"),
                   32);

    EXPECT_EQ(initial_salt_server.to_hex(),
              "fda3953aecc040e48b34e27ef87de3a6");

    String output =
        hkdf_expand_label(initial_salt_server,
                          DCID,
                          String::from_text("quic hp"),
                          16);

    EXPECT_EQ(output.size(), 16);

    EXPECT_EQ(output.to_hex(),
              "a8ed82e6664f865aedf6106943f95fb8");
}

TEST_F(CryptoTest, ClientHPSecret) {
    String output =
        hkdf_expand_label( client_initial_secret,
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

TEST_F(CryptoTest, DecodeHeaderProtection) {
}

} // namespace crypto

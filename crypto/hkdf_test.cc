#include "gtest/gtest.h"
#include "hkdf.h"

namespace crypto {

class HkdfTest : public ::testing::Test {
protected:
    HkdfTest() = default;
    ~HkdfTest() override = default;

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

TEST_F(HkdfTest, Test01) {
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

TEST_F(HkdfTest, ClientInSecret) {

    String output =
        hkdf_label(initial_salt,
                   DCID,
                   String::from_text("client in"),
                   32);

    EXPECT_EQ(output.size(), 32);

    EXPECT_EQ(output.to_hex(),
              client_initial_secret.to_hex());
}

TEST_F(HkdfTest, ClientHPSecret) {
    String output =
        hkdf_expand_label( client_initial_secret,
                           String::from_text("quic hp"),
                           String::empty_string(),
                           16);

    EXPECT_EQ(output.size(), 16);

    EXPECT_EQ(output.to_hex(),
              client_hp.to_hex());
}

} // namespace crypto

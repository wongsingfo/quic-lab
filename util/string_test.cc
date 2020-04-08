#include "gtest/gtest.h"
#include "util/string.h"

class StringTest : public ::testing::Test {
protected:
    StringTest() = default;
    ~StringTest() override = default;

    void SetUp() override {
    }

    void TearDown() override {
    };
};

TEST_F(StringTest, Test01) {
    String s("\001\002\003", 3);
    EXPECT_EQ(s.to_hex(), "010203");
}

TEST_F(StringTest, Test02) {
    String s = String::from_hex("ff0188");
    EXPECT_EQ(s.size(), 3);
    EXPECT_EQ(s.data()[2], (unsigned char) '\x88');
}

TEST_F(StringTest, Test03) {
    String s("\001\000\002\000", 4);
    EXPECT_EQ(s.to_hex(), "01000200");
}

TEST_F(StringTest, Test04) {
    String s = String::from_hex("0x12 0x1314");
    EXPECT_EQ(s.to_hex(), "121314");
}

TEST_F(StringTest, Test05) {
    EXPECT_ANY_THROW(String::from_hex("0x12r 0x13"));
}

TEST_F(StringTest, Test06) {
    EXPECT_EQ(
        String("123", 3).to_hex(),
        String::from_text("123").to_hex()
    );
}


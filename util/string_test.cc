#include "gtest/gtest.h"

#include "util/string.h"
#include "util/string_reader.h"

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

class StringReaderTest : public ::testing::Test {
protected:
    StringReaderTest() = default;
    ~StringReaderTest() override = default;

    void SetUp() override {
    }

    void TearDown() override {
    };
};

TEST_F(StringReaderTest, Test01) {
    String input = String::from_hex("c2 19 7c 5e ff 14 e8 8c");
    StringReader reader(input);
    EXPECT_EQ(
        reader.read_with_variant_length(),
        151288809941952652
    );
}

TEST_F(StringReaderTest, Test02) {
    String input = String::from_hex("25");
    StringReader reader(input);
    EXPECT_EQ(
        reader.read_with_variant_length(),
        37
    );
}

TEST_F(StringReaderTest, Test03) {
    String input = String::from_hex("40 25");
    StringReader reader(input);
    EXPECT_EQ(
        reader.read_with_variant_length(),
        37
    );
}

TEST_F(StringReaderTest, Test04) {
    String input = String::from_hex("25");
    StringReader reader(input);
    EXPECT_EQ(
        reader.read_with_variant_length(),
        37
    );
}


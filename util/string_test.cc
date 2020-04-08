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
    EXPECT_EQ(s.to_string(), "010203");
}

TEST_F(StringTest, Test02) {
    String s = String::from_text("ff0188");
    EXPECT_EQ(s.size(), 3);
    EXPECT_EQ(s.data()[2], u'\x88');
}

TEST_F(StringTest, Test03) {
    String s("\001\000\002", 3);
    EXPECT_EQ(s.to_string(), "010002");
}


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
};

TEST_F(HkdfTest, Test01) {
    EXPECT_FALSE(false);
}

}

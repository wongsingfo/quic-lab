//
// Created by Chengke Wong on 2020/4/29.
//

#include "gtest/gtest.h"
#include "util/string.h"
#include "transport/recv_buffer.h"

class RecvBufferTest : public ::testing::Test {

protected:
    RecvBufferTest() = default;

    ~RecvBufferTest() override = default;

    void SetUp() override {
    }

    void TearDown() override {
    }

    void data_arrive(uint64_t begin, uint64_t end) {
        assert(begin <= end);
        buffer.insert(begin, StringRef(&data[begin], end - begin));
    }

    void read_data(size_t size, size_t expect_read) {
        String data_read(size);
        EXPECT_EQ(buffer.read_to_end(data_read), expect_read);
        EXPECT_EQ(data_read.sub_string(0, expect_read),
                  data.sub_string(offset, offset + expect_read));
        offset += expect_read;
    }

    void read_data(size_t size) {
        read_data(size, size);
    }

    RecvBuffer buffer;
    String data = String::random(100);
    size_t offset = 0;
};

TEST_F(RecvBufferTest, Test01) {
    EXPECT_EQ(buffer.retired(), 0);
    data_arrive(1, 5);
    EXPECT_EQ(buffer.retired(), 0);
    EXPECT_FALSE(buffer.readable());
    data_arrive(0, 2);
    EXPECT_TRUE(buffer.readable());
    read_data(5);
    EXPECT_EQ(buffer.retired(), 5);
}

TEST_F(RecvBufferTest, Test02) {
    data_arrive(1, 5);
    data_arrive(7, 10);
    read_data(10, 0);
    data_arrive(5, 7);
    read_data(10, 0);
    data_arrive(0, 1);
    read_data(10);
}

TEST_F(RecvBufferTest, Test03) {
    data_arrive(1, 5);
    data_arrive(7, 10);
    read_data(10, 0);
    data_arrive(0, 11);
    read_data(10);
    EXPECT_EQ(buffer.retired(), 10);
    read_data(1);
    EXPECT_EQ(buffer.retired(), 11);
}

TEST_F(RecvBufferTest, Test04) {
    data_arrive(0, 50);
    read_data(50);
    EXPECT_EQ(buffer.retired(), 50);
}

TEST_F(RecvBufferTest, Test05) {
    for (size_t i = 0; i < 2; i++) {
        data_arrive(i, i + 2);
        read_data(1);
        EXPECT_EQ(buffer.retired(), i + 1);
    }
}


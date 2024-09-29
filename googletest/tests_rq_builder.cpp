#include <cstring>
#include "gtest/gtest.h"
#include "../src/http/RequestBuilder.h"

//TEST(TestSuiteName, TestName)
TEST(RQBuilderTest1, Positive) {
    http::RequestBuilder rq1 = http::RequestBuilder();
    rq1.buf().resize(100);
    std::memcpy(rq1.buf().data(), "GET / HTTP/1.1\r\n", 16);
    // maybe fork() and run loop in child to catch possible infinite loop (with timeout)
    while (!rq1.IsReadyForResponse()) {
        rq1.ParseNext();
    }
    EXPECT_EQ(http::HTTP_GET, rq1.rq().method);
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

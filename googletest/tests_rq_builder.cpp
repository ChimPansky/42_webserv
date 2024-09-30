#include <cstring>
#include "gtest/gtest.h"
#include "../src/http/RequestBuilder.h"

//TEST(TestSuiteName, TestName)
TEST(RQBuilderTest1, Positive) {
    http::RequestBuilder rq1 = http::RequestBuilder();
    rq1.buf().resize(100);
    std::string raw_req = "GET /index.html HTTP/1.1\r\nKey: Value\r\nHost:localhost\r\nUser-Agent: curl/7.68.0\r\nAccept: */*\r\n\r\n";
    std::memcpy(rq1.buf().data(), raw_req.data(), raw_req.size());
    // maybe fork() and run loop in child to catch possible infinite loop (with timeout)
    while (!rq1.IsReadyForResponse()) {
        rq1.ParseNext();
    }
    EXPECT_EQ(http::HTTP_GET, rq1.rq().method);
    EXPECT_EQ("localhost", rq1.rq().GetHeaderVal("Host"));
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

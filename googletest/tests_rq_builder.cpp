#include <cstring>
#include "gtest/gtest.h"
#include "../src/http/RequestBuilder.h"
#include <fstream>

size_t read_size = 10;

size_t ReadFromFile(std::ifstream& file, std::vector<char>& buf, size_t sz) {
    size_t old_sz = buf.size();
    buf.resize(old_sz + sz);
    file.read(buf.data() + old_sz, sz);
    return file.gcount();
}
//TEST(TestSuiteName, TestName)
TEST(RQBuilderTest1, Positive) {
    http::RequestBuilder rqb = http::RequestBuilder();
    std::ifstream file("requests/rq1.txt");
    if (!file.is_open()) {
        std::cerr << "File not found" << std::endl;
        return;
    }
    while (!rqb.IsReadyForResponse() && ReadFromFile(file, rqb.buf(), read_size) > 0) {
        rqb.ParseNext();
    }
    EXPECT_EQ(http::HTTP_GET, rqb.rq().method);
    EXPECT_EQ("localhost", rqb.rq().GetHeaderVal("Host"));
}

TEST(RQBuilderTest2, Positive) {
    http::RequestBuilder rqb = http::RequestBuilder();
    std::string raw_req = "GET /index.html HTTP/1.1\r\nKey: Value\r\nHost:localhost\r\nUser-Agent: curl/7.68.0\r\nAccept: */*\r\n\r\n";
    rqb.buf().resize(raw_req.size());
    std::memcpy(rqb.buf().data(), raw_req.data(), raw_req.size());
    // maybe fork() and run loop in child to catch possible infinite loop (with timeout)
    while (!rqb.IsReadyForResponse()) {
        rqb.ParseNext();
    }
    EXPECT_EQ(http::HTTP_GET, rqb.rq().method);
    EXPECT_EQ("localhost", rqb.rq().GetHeaderVal("Host"));
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

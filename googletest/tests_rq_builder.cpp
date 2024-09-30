#include <cstring>
#include "gtest/gtest.h"
#include "../src/http/RequestBuilder.h"
#include <fstream>

size_t read_size = 10;

static size_t ReadFromFile(std::ifstream& file, std::vector<char>& buf, size_t sz) {
    size_t old_sz = buf.size();
    buf.resize(old_sz + sz);
    file.read(buf.data() + old_sz, sz);
    return file.gcount();
}

static int BuildRequest(http::RequestBuilder& builder, const char* rq_path) {
    std::ifstream file(rq_path);
    if (!file.is_open()) {
        std::cerr << "Could not open Request File: " << rq_path << std::endl;
        return 1;
    }
    while (!builder.IsReadyForResponse() && ReadFromFile(file, builder.buf(), read_size) > 0) {
        builder.ParseNext();
    }
    return 0;
}

//TEST(TestSuiteName, TestName)
TEST(Suite0, Positive) {
    http::RequestBuilder builder = http::RequestBuilder();
    std::string raw_req = "GET /index.html HTTP/1.1\r\nKey: Value\r\nHost:localhost\r\nUser-Agent: curl/7.68.0\r\nAccept: */*\r\n\r\n";
    builder.buf().resize(raw_req.size());
    std::memcpy(builder.buf().data(), raw_req.data(), raw_req.size());
    // maybe fork() and run loop in child to catch possible infinite loop (with timeout)
    while (!builder.IsReadyForResponse()) {
        builder.ParseNext();
    }
    EXPECT_EQ(http::HTTP_GET, builder.rq().method);
    EXPECT_EQ("localhost", builder.rq().GetHeaderVal("Host"));
}

TEST(Suite1, Test1) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq1.txt") != 0) {
        FAIL();
    }
    EXPECT_EQ(true, builder.rq().complete);
    EXPECT_EQ(false, builder.rq().bad_request);
    EXPECT_EQ(http::HTTP_GET, builder.rq().method);
    EXPECT_EQ("/index.html", builder.rq().uri);
    EXPECT_EQ(http::HTTP_2, builder.rq().version);
    EXPECT_EQ("192.168.1.1", builder.rq().GetHeaderVal("Host"));
    EXPECT_EQ("*/*", builder.rq().GetHeaderVal("Accept"));
}

TEST(Suite1, Test2) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq2.txt") != 0) {
        FAIL();
    }
    EXPECT_EQ(true, builder.rq().complete);
    EXPECT_EQ(false, builder.rq().bad_request);
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/", builder.rq().uri);
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("localhost", builder.rq().GetHeaderVal("Host"));
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

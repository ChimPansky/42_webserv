#include <cstring>
#include "gtest/gtest.h"
#include "../src/http/RequestBuilder.h"
#include <fstream>

size_t read_size = 10;

#define BODY_14 "Hello, World!\n"

#define BODY_1097 "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptat"

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
    size_t bytes_read = 0;
    while (!builder.IsReadyForResponse()) {
        bytes_read = ReadFromFile(file, builder.buf(), read_size);
        std::cout << "Tester: bytes_read: " << bytes_read << std::endl;
        builder.buf().resize(builder.buf().size() - (read_size - bytes_read));
        builder.ParseNext(bytes_read);
        if (bytes_read == 0) {
            break;
        }
    }
    return 0;
}

//TEST(TestSuiteName, TestName)
// TEST(Suite0, Test0) {
//     http::RequestBuilder builder = http::RequestBuilder();
//     std::string raw_req = "GET /index.html HTTP/1.1\r\nKey: Value\r\nHost:localhost\r\nUser-Agent: curl/7.68.0\r\nAccept: */*\r\n\r\n";
//     builder.buf().resize(raw_req.size());
//     std::memcpy(builder.buf().data(), raw_req.data(), raw_req.size());
//     // maybe fork() and run loop in child to catch possible infinite loop (with timeout)
//     while (!builder.IsReadyForResponse()) {
//         builder.ParseNext();
//     }
//     EXPECT_EQ(http::HTTP_GET, builder.rq().method);
//     EXPECT_EQ("localhost", builder.rq().GetHeaderVal("Host"));
// }

TEST(Suite1, Test1) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq1.txt") != 0) {
        FAIL();
    }
    EXPECT_EQ(false, builder.rq().rq_complete);
    EXPECT_EQ(true, builder.rq().bad_request);
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/", builder.rq().uri);
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("bla", builder.rq().GetHeaderVal("host"));
    EXPECT_EQ("14", builder.rq().GetHeaderVal("content-length"));
    EXPECT_EQ(14, builder.rq().body.content.size());
    EXPECT_EQ(8, builder.rq().body.content_idx);
    // EXPECT_EQ(BODY_1097, builder.rq().body.content.data());
    //EXPECT_EQ(BODY_14, builder.rq().body.content.data());
    // EXPECT_EQ("chunked", builder.rq().GetHeaderVal("transfer-encoding"));
    // EXPECT_EQ(true, builder.rq().body.chunked);

    // EXPECT_EQ("192.168.1.1", builder.rq().GetHeaderVal("Host"));
    // EXPECT_EQ("*/*", builder.rq().GetHeaderVal("Accept"));
}

// TEST(Suite1, Test2) {
//     http::RequestBuilder builder;
//     if (BuildRequest(builder, "requests/rq2.txt") != 0) {
//         FAIL();
//     }
//     EXPECT_EQ(true, builder.rq().complete);
//     EXPECT_EQ(false, builder.rq().bad_request);
//     EXPECT_EQ(http::HTTP_POST, builder.rq().method);
//     EXPECT_EQ("/", builder.rq().uri);
//     EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
//     EXPECT_EQ("localhost", builder.rq().GetHeaderVal("Host"));
// }

// TEST(Suite1, Test3) {
//     http::RequestBuilder builder;
//     if (BuildRequest(builder, "requests/rq3.txt") != 0) {
//         FAIL();
//     }
//     EXPECT_EQ(true, builder.rq().complete);
//     EXPECT_EQ(false, builder.rq().bad_request);
//     EXPECT_EQ(http::HTTP_GET, builder.rq().method);
//     EXPECT_EQ("/", builder.rq().uri);
//     EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
//     EXPECT_EQ("www.example.com", builder.rq().GetHeaderVal("Host"));
//     EXPECT_EQ("Mozilla/5.0 (Windows NT 10.0; Win64; x64)", builder.rq().GetHeaderVal("User-Agent"));
//     EXPECT_EQ("text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8", builder.rq().GetHeaderVal("Accept"));
//     EXPECT_EQ("en-US,en;q=0.5", builder.rq().GetHeaderVal("Accept-Language"));
//     EXPECT_EQ("gzip, deflate, br", builder.rq().GetHeaderVal("Accept-Encoding"));
//     EXPECT_EQ("keep-alive", builder.rq().GetHeaderVal("Connection"));
// }

// TEST(Suite1, Test4) {
//     http::RequestBuilder builder;
//     if (BuildRequest(builder, "requests/rq4.txt") != 0) {
//         FAIL();
//     }
//     builder.rq().Print();
//     EXPECT_EQ(true, builder.rq().complete);
//     EXPECT_EQ(false, builder.rq().bad_request);
//     EXPECT_EQ(http::HTTP_POST, builder.rq().method);
//     EXPECT_EQ("/submit-form", builder.rq().uri);
//     EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
//     EXPECT_EQ("www.example.com", builder.rq().GetHeaderVal("Host"));
//     EXPECT_EQ("Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36", builder.rq().GetHeaderVal("User-Agent"));
//     EXPECT_EQ("application/json, text/javascript, */*; q=0.01", builder.rq().GetHeaderVal("Accept"));
//     EXPECT_EQ("en-US,en;q=0.9", builder.rq().GetHeaderVal("Accept-Language"));
//     EXPECT_EQ("gzip, deflate, br", builder.rq().GetHeaderVal("Accept-Encoding"));
//     EXPECT_EQ("application/x-www-form-urlencoded", builder.rq().GetHeaderVal("Content-Type"));
//     EXPECT_EQ("27", builder.rq().GetHeaderVal("Content-Length"));
//     EXPECT_EQ("keep-alive", builder.rq().GetHeaderVal("Connection"));
//
    // If you need to check the request body as well:
    //EXPECT_EQ("name=JohnDoe&age=28", builder.rq().body);
// }

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

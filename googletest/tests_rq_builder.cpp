#include <cstring>
#include "gtest/gtest.h"
#include "../src/http/RequestBuilder.h"
#include <fstream>

#define BODY_14 "Hello, World!!"

#define BODY_1100 "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptat!!!"

static size_t ReadFromFile(std::ifstream& file, std::vector<char>& buf, size_t sz) {
    size_t old_sz = buf.size();
    buf.resize(old_sz + sz);
    file.read(buf.data() + old_sz, sz);
    return file.gcount();
}

static int BuildRequest(http::RequestBuilder& builder, const char* rq_path, size_t read_size = 10) {
    std::ifstream file(rq_path);
    if (!file.is_open()) {
        std::cerr << "Could not open Request File: " << rq_path << std::endl;
        return 1;
    }
    size_t bytes_read = 0;
    while (!builder.IsReadyForResponse()) {
        bytes_read = 0;
        if (builder.needs_info_from_server()) {
            //std::cout << "RequestBuilder needs info from server -> Get it and then continue building..." << std::endl;
            // these settings should come from server:
            builder.set_max_body_size(10000);  // 1MB
        }
        if (builder.HasReachedEndOfBuffer()) {
            bytes_read = ReadFromFile(file, builder.buf(), read_size);
            builder.buf().resize(builder.buf().size() - (read_size - bytes_read));
        }
        if (builder.ParseNext(bytes_read) == 0) {
            std::cerr << "ParseNext returned 0..." << std::endl;
            break;
        }
    }
    return 0;
}

TEST(ValidWithBody, 1_Bodylen_14_Buffer_50) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq1.txt", 50) != 0) {
        FAIL();
    }
    EXPECT_TRUE(builder.rq().rq_complete);
    EXPECT_FALSE(builder.rq().bad_request);
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/", builder.rq().uri);
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("www.example.com", builder.rq().GetHeaderVal("host"));
    EXPECT_EQ("14", builder.rq().GetHeaderVal("content-length"));
    EXPECT_EQ(14, builder.rq().body.content.size());
    EXPECT_STREQ(BODY_14, builder.rq().body.content.data());
}

TEST(ValidWithBody, 2_Chunked_1100_Buffer_10) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq2.txt", 11) != 0) {
        FAIL();
    }
    EXPECT_TRUE(builder.rq().rq_complete);
    EXPECT_FALSE(builder.rq().bad_request);
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/upload", builder.rq().uri);
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("chunked", builder.rq().GetHeaderVal("transfer-encoding"));
    EXPECT_STREQ(BODY_1100, builder.rq().body.content.data());
}

TEST(ValidWithBody, 3_Chunked_1100_Buffer_9) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq3.txt", 9) != 0) {
        FAIL();
    }
    EXPECT_TRUE(builder.rq().rq_complete);
    EXPECT_FALSE(builder.rq().bad_request);
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/upload", builder.rq().uri);
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("chunked", builder.rq().GetHeaderVal("transfer-encoding"));
    EXPECT_STREQ(BODY_1100, builder.rq().body.content.data());
}

TEST(ValidWithBody, 4_Bodylen_1_Buffer_50) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq4.txt", 50) != 0) {
        FAIL();
    }
    EXPECT_TRUE(builder.rq().rq_complete);
    EXPECT_FALSE(builder.rq().bad_request);
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/", builder.rq().uri);
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("www.example.com", builder.rq().GetHeaderVal("host"));
    EXPECT_EQ("1", builder.rq().GetHeaderVal("content-length"));
    EXPECT_EQ(1, builder.rq().body.content.size());
    EXPECT_STREQ("a", builder.rq().body.content.data());
}

TEST(ValidWithBody, 5_Chunked_1_Buffer_10) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq5.txt", 50) != 0) {
        FAIL();
    }
    EXPECT_TRUE(builder.rq().rq_complete);
    EXPECT_FALSE(builder.rq().bad_request);
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/uploa", builder.rq().uri);
    EXPECT_EQ(http::HTTP_1_0, builder.rq().version);
    EXPECT_EQ("chunked", builder.rq().GetHeaderVal("transfer-encoding"));
    EXPECT_STREQ("L", builder.rq().body.content.data());
}

// Valid without body:
TEST(ValidWithoutBody, 6_SimpleGet_Buffer_50) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq6.txt", 50) != 0) {
        FAIL();
    }
    EXPECT_TRUE(builder.rq().rq_complete);
    EXPECT_FALSE(builder.rq().bad_request);
    EXPECT_EQ(http::HTTP_GET, builder.rq().method);
    EXPECT_EQ("/", builder.rq().uri);
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("www.example.com", builder.rq().GetHeaderVal("host"));
    EXPECT_EQ("", builder.rq().GetHeaderVal("content-length"));  // No body, no content-length
    EXPECT_TRUE(builder.rq().body.content.empty());
}


TEST(ValidWithoutBody, 7_GetWithQuery_Buffer_100) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq7.txt", 100) != 0) {
        FAIL();
    }
    EXPECT_TRUE(builder.rq().rq_complete);
    EXPECT_FALSE(builder.rq().bad_request);
    EXPECT_EQ(http::HTTP_GET, builder.rq().method);
    EXPECT_EQ("/search?q=example", builder.rq().uri);
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("www.search.com", builder.rq().GetHeaderVal("host"));
    EXPECT_EQ("", builder.rq().GetHeaderVal("content-length"));  // No body, no content-length
    EXPECT_TRUE(builder.rq().body.content.empty());
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

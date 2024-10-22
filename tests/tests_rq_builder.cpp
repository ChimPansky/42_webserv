#include <cstring>
#include "gtest/gtest.h"
#include "../src/http/RequestBuilder.h"
#include <fstream>
#include <iostream>
#define BODY_14 "Hello, World!!"

#define BODY_1100 "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptat!!!"

#define CLIENT_MAX_BODY_SIZE 1500

ssize_t Recv(std::ifstream& file, std::vector<char>& buf, size_t read_sz) {
    // size_t old_buf_sz = buf.size();
    // buf.resize(old_buf_sz + read_sz);
    file.read(buf.data() + buf.size() - read_sz, read_sz);
    // size_t bytes_recvd = file.gcount();
    // if (bytes_recvd >= 0 && static_cast<size_t>(bytes_recvd) < read_sz) {
    //     buf.resize(old_buf_sz + bytes_recvd);
    // }
    return file.gcount();
}

// Client Context
void ProcessNewData(http::RequestBuilder& builder, size_t bytes_recvd) {
    builder.Build(bytes_recvd);
    if (builder.builder_status() == http::RB_NEED_INFO_FROM_SERVER) {
        builder.ApplyServerInfo(CLIENT_MAX_BODY_SIZE);
        builder.Build(bytes_recvd);
    }
}

// Client Callback context
bool Call(http::RequestBuilder& builder, std::ifstream& file, size_t read_sz) {
    builder.PrepareToRecvData(read_sz);
    size_t bytes_recvd = Recv(file, builder.buf(), read_sz);
    if (bytes_recvd < 0) {
        std::cerr << "Could not read from client: closing connection..." << std::endl;
        //client_.CloseConnection();
        return false;
    }
    builder.AdjustBufferSize(bytes_recvd);
    ProcessNewData(builder, bytes_recvd);
    return true;
}

int BuildRequest(http::RequestBuilder& builder, const char* rq_path, size_t read_size = 10) {
    std::ifstream file(rq_path);
    if (!file.is_open()) {
        std::cerr << "Could not open Request File: " << rq_path << std::endl;
        return 1;
    }
    while (builder.builder_status() != http::RB_DONE) {
        if (!Call(builder, file, read_size)) {
            break;
        };
    }
    return 0;
}

TEST(ValidWithBody, 1_Bodylen_14) {
    http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq1.txt", 50) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("www.example.com", builder.rq().GetHeaderVal("host").second);
    ASSERT_EQ("14", builder.rq().GetHeaderVal("content-length").second);
    const char* str = BODY_14;
    ASSERT_EQ(std::vector<char>(str, str + strlen(str)), builder.rq().body);
    ASSERT_EQ((unsigned long)14, builder.rq().body.size());
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

TEST(ValidWithBody, 2_One_Chunk_1100) {
    http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq2.txt", 10) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/upload", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("chunked", builder.rq().GetHeaderVal("transfer-encoding").second);
    ASSERT_EQ((unsigned long)1100, builder.rq().body.size());
    const char* str = BODY_1100;
    ASSERT_EQ(std::vector<char>(str, str + strlen(str)), builder.rq().body);
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

TEST(ValidWithBody, 3_One_Chunk_1100) {
    http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq2.txt", 9) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/upload", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("chunked", builder.rq().GetHeaderVal("transfer-encoding").second);
    ASSERT_EQ((unsigned long)1100, builder.rq().body.size());
    const char* str = BODY_1100;
    ASSERT_EQ(std::vector<char>(str, str + strlen(str)), builder.rq().body);
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

TEST(ValidWithBody, 4_Bodylen_1) {
    http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq4.txt", 50) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("www.example.com", builder.rq().GetHeaderVal("host").second);
    ASSERT_EQ("1", builder.rq().GetHeaderVal("content-length").second);
    ASSERT_EQ((unsigned long)1, builder.rq().body.size());
    const char *str = "a";
    ASSERT_EQ(std::vector<char>(str, str + strlen(str)), builder.rq().body);
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

TEST(ValidWithBody, 5_Chunked_1) {
    http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq5.txt", 50) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/upload", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_0, builder.rq().version);
    ASSERT_EQ("chunked", builder.rq().GetHeaderVal("transfer-encoding").second);
    const char *str = "L";
    ASSERT_EQ(std::vector<char>(str, str + strlen(str)), builder.rq().body);
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

// Valid without body:
TEST(ValidWithoutBody, 6_SimpleGet) {
    http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq6.txt", 50) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("www.example.com", builder.rq().GetHeaderVal("host").second);
    ASSERT_EQ("", builder.rq().GetHeaderVal("content-length").second);
    ASSERT_TRUE(builder.rq().body.empty());
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

TEST(ValidWithoutBody, 7_GetWithQuery) {
    http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq7.txt", 100) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/search?q=example", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("www.search.com", builder.rq().GetHeaderVal("host").second);
    ASSERT_EQ("", builder.rq().GetHeaderVal("content-length").second);
    ASSERT_TRUE(builder.rq().body.empty());
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

TEST(ValidWithoutBody, 8_GetWithHeaders) {
    http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq8.txt", 100) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/products", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("shop.example.com", builder.rq().GetHeaderVal("host").second);
    ASSERT_EQ("application/json", builder.rq().GetHeaderVal("accept").second);
    ASSERT_EQ("CustomClient/1.0", builder.rq().GetHeaderVal("user-agent").second);
    ASSERT_TRUE(builder.rq().body.empty());
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

TEST(InValidWithoutBody, 9_PostWithHeaders) {
    http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq9.txt", 80) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/submit", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("www.example.com", builder.rq().GetHeaderVal("host").second);
    ASSERT_EQ("application/x-www-form-urlencoded", builder.rq().GetHeaderVal("content-type").second);
    ASSERT_EQ("http://www.example.com", builder.rq().GetHeaderVal("referer").second);
    ASSERT_TRUE(builder.rq().body.empty());
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(ValidWithoutBody, 10_DeleteWithHeaders) {
    http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq10.txt", 50) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_DELETE, builder.rq().method);
    ASSERT_EQ("/items/123", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_0, builder.rq().version);
    ASSERT_EQ("api.items.com", builder.rq().GetHeaderVal("host").second);
    ASSERT_EQ("Bearer some_token", builder.rq().GetHeaderVal("authorization").second);
    ASSERT_TRUE(builder.rq().body.empty());
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

TEST(InValidWithoutBody, 11_Incomplete_Method) {
    http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq11.txt", 7) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 12_Too_Many_Spaces) {
    http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq12.txt", 7) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 13_No_URI) {
     http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq13.txt", 10) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_TRUE(builder.rq().uri.empty());
    ASSERT_EQ(http::HTTP_NO_VERSION, builder.rq().version);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 14_No_Invalid_Version) {
     http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq14.txt", 1000) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/upload", builder.rq().uri);
    ASSERT_EQ(http::HTTP_NO_VERSION, builder.rq().version);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

// when testing this and viewing rq15.txt in editor: careful about VS Code setting "Files: Insert Final Newline"
TEST(InValidWithoutBody, 15_No_CRLF_After_Version) {
     http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq15.txt", 10000) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_DELETE, builder.rq().method);
    ASSERT_EQ("/upload/subfolder1/subfolder2", builder.rq().uri);
    ASSERT_EQ(http::HTTP_NO_VERSION, builder.rq().version);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 16_Just_LF_After_Version) {
     http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq16.txt", 13) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_DELETE, builder.rq().method);
    ASSERT_EQ("/upload/subfolder1/subfolder2", builder.rq().uri);
    ASSERT_EQ(http::HTTP_NO_VERSION, builder.rq().version);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 17_Bad_Header_Key) {
     http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq17.txt", 9) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_DELETE, builder.rq().method);
    ASSERT_EQ("/upload/subfolder1/subfolder2", builder.rq().uri);
    ASSERT_EQ(http::HTTP_2, builder.rq().version);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 18_Bad_Header_Key) {
     http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq18.txt", 1) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_DELETE, builder.rq().method);
    ASSERT_EQ("/upload/subfolder1/subfolder2", builder.rq().uri);
    ASSERT_EQ(http::HTTP_2, builder.rq().version);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 19_Bad_Header_Key) {
     http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq19.txt", 100) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_FALSE(builder.rq().GetHeaderVal("Host!").first);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 20_Bad_Header_Key) {
    http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq20.txt", 100) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_FALSE(builder.rq().GetHeaderVal("Host-").first);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 21_Bad_Header_Key) {
     http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq21.txt", 100) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_FALSE(builder.rq().GetHeaderVal("Host ").first);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 22_Missing_Space) {
     http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq22.txt", 70) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_FALSE(builder.rq().GetHeaderVal("host").first);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 23_Missing_Header_Value) {
     http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq23.txt", 70) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_FALSE(builder.rq().GetHeaderVal("host").first);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 24_No_CRLF_After_Header_Value) {
     http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq24.txt", 10) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_FALSE(builder.rq().GetHeaderVal("host").first);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

//TEST(InValidWithoutBody, 25_Bad_Header_Value_10) {
// TODO: Tests 25-30 check headervalues with invalid characters


//TODO: Tests 31+ check for conflicting headers? multiple headers with same key?


//TODO: Tests 41+ check for invalid body content

TEST(InValidWithBody, 50_Bad_Chunk_size_has_plus) {
     http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq50.txt", 1000) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("chunked", builder.rq().GetHeaderVal("Transfer-Encoding").second);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithBody, 51_Bad_Chunk_size_has_minus) {
     http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq51.txt", 1000) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("chunked", builder.rq().GetHeaderVal("Transfer-Encoding").second);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithBody, 52_Bad_Chunk_size_has_leading_spaces) {
     http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq52.txt", 1000) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("chunked", builder.rq().GetHeaderVal("Transfer-Encoding").second);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithBody, 53_Bad_Chunk_size_has_trailing_spaces) {
     http::RequestBuilder builder = http::RequestBuilder();
    if (BuildRequest(builder, "requests/rq51.txt", 1000) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("chunked", builder.rq().GetHeaderVal("Transfer-Encoding").second);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
